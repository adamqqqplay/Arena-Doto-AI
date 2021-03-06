# Defense of the Origin

[TOC]

## 前言

本游戏的灵感来自于DOTA，因此我将它命名为DOTO，表示还有代码中会有很多TODO

## 游戏介绍 简略版

本游戏是多人即时对战类游戏，每局游戏会生成一个存在若干矩形障碍物的矩形场地，多名角色诞生在场地中，每个角色属于一个势力，一名玩家只能控制自己势力的角色，支持玩家与玩家之间使用火球术与陨石术互相攻击。场地中每个势力会生成一个魔法水晶，游戏目标是抢夺属于别人的魔法水晶，并将魔法水晶运送到自己的阵地内，获得得分；或是杀死其他势力的角色，获得得分；或是吃掉生成的道具。最终根据每个玩家所控制的势力的得分进行排名。

## 游戏介绍

### 时间机制

本游戏是即时对战类游戏，即你写的ai需要根据当前的局面做出一定的决策，为了给予选手充分的时间进行决策的思考，我们做出如下两点改动

1. 每一秒有`frames_per_second`帧，该常数值小于24，因此，观看的ai对决的replay视频渲染时间要大于实际时长
2. 我们保证提供给选手的地图信息是在传递参数时最新的参数，但是选手不一定要每一帧都做出及时的反馈，我们只会在指定的时间处监听选手的决策，若选手没有及时传递回决策，按照未做出任何决策处理。支持选手花费超过一帧的时间做出一次决策，但是在同一帧的时间内做出多次决策以最后一次为准

### 势力

每个玩家控制一个势力下若干角色，共有`faction_number`个势力，每个势力可以操纵`human_number`个角色，第i个势力操纵的第k个角色的编号为`k*faction_number+i`，其中`0<=k<human_number`

### 地图

初始时，生成一张大小为`width*height`的地图，使用大小为`room_size`的尺寸进行网格化，部分网格里面有障碍物，且障碍物（墙）不会导致地图上非障碍物部分不连通。地图中每个角色有固定的出生/复活地点（同一地点），调用`birth_palces[i][j]`可以查看势力i操纵的第j个角色的出生地点，每个魔法水晶有固定的初始地点，第i个水晶的初始地点在`ball_places[i]`，每个势力有自己的目标地点，第i个势力的目标地点是以`target_places[i]`为中心，半径为`target_radius`的圆，还有若干道具生成地点`bonus_places[i]`，每隔随机时间`bonus_time_low~bonus_time_high`会生成一个道具

**以下只讨论存活的角色，处于死亡状态的角色没有任何影响**

### 水晶

魔法水晶是得分的两种方式之一，水晶可以看做图中的一个点。全地图共有`faction_number`个水晶，所属于不同的势力，在初始地点生成。水晶还有一个属性表示当前持有水晶的角色是谁，无主的水晶该属性为-1。水晶不能被自己势力的角色持有，且一旦被某个角色持有后水晶的位置与该角色保持相同，其他任何角色都不能再持有该水晶直到水晶掉落或水晶重置。当持有水晶的角色死亡后，水晶掉落在原地；当持有水晶的角色送到该角色势力的目标区域后，水晶重置状态回到初始位置，持有人变为-1，分数机制更新。当水晶处于无主状态时，会自动由以该水晶位置为中心，`ball_radius`为半径区域内离水晶最近的，可以持有该水晶的角色持有，找不到这样的角色就无变化。

### 角色的死亡、复活与移动

每个角色满状态时有`human_hp`的血量，在受到伤害后会掉血，血量小于等于0时角色会死亡，死亡后会在`frames_of_death`帧之后以满状态复活（满状态是指满血量，攻击冷却时间归零，所有法术冷却时间归零，所有法术使用次数恢复上限），复活地点为与之绑定的复活地点，在复活后，角色拥有`frames_of_invincible`帧的无敌时间（不受任何伤害）角色的出生可以看做开局进行了一次复活。

每个角色每帧可以一个指定移动的地点，该地点要求在地图范围内，不在墙区域，距离该角色当前位置不超过`human_velocity`，可以看做闪烁到新的位置

### 攻击与法术

普通攻击方式有一种，法术有两种

普通攻击（火球术）：在角色指定方向离角色`splash_radius`远生成一个火球（如果该位置在地图内，且不在墙的范围内），火球可以看成一个点，向前以每帧`fireball_velocity`的移动速度飞去，在碰到墙（火球一点点闪现到了障碍物的范围内）/地图边界（地图边界外）/角色（碰到角色的判定方式不同于墙和边界，为火球与角色的距离小于`fireball_radius`，注意此处判定为每帧结束时火球的位置与人的位置之间的距离）才会消失，**只要火球消失**，就会在该位置造成半径为`splash_radius`(略大于`fireball_radius`)范围内的伤害`splash_hurt`。火球不可以连发，有`human_fireball_interval`的冷却帧数限制。

陨石术：指定角色的半径`meteor_distance`内一点（可以为障碍物上，不可以在边界外），放置一个陨石轰击指示物，经过`meteor_delay`的帧数之后，天降一颗陨石，对半径为`explode_radius`内所有角色造成`explode_hurt`的伤害。陨石术不可以连发，有`human_meteor_interval`的冷却帧数限制，陨石术不能无限次发射，每个角色出生/复活后可以使用`human_meteor_number`次陨石术，使用一次少一次。

闪现：指定角色的半径`flash_distance`内一点（地图内，非墙区域），瞬间移动到该位置。闪现不可以连发，有`human_flash_interval`的冷却帧数限制，闪现不能无限次发射，每个角色出生/复活后可以使用`human_flash_number`次闪现，使用一次少一次。**注意：拿到球的角色不能使用闪现！**

## 道具

当前道具生成点没有道具时，会生成一个得分道具，任意角色到达以指定点为中心，半径为`bonus_radius`的圆内都可以直接得分，当道具被吃掉后，会随机一段时间之后再次生成，时间为`bonus_time_low~bonus_time_high`

### 分数机制

有三种得分方法

1. 角色A放出陨石术或火球术杀死角色B，A所属势力得分`kill_score`，B所属势力得分`killed_score`（非正），注意有可能在B角色死亡帧内有多角色发动了攻击，那么`kill_score`根据贡献分摊
2. 角色A携带了势力b的水晶进入了自己所属势力a的目标区域，a得分`goal_score`，b得分`goaled_score`（非正）
3. 角色A在道具生成地点(以指定点为中心，半径为`bonus_radius`的圆)吃到了道具，自己所属势力a得分`bonus_score`

## 游戏判定机制

每帧按照一下顺序进行判定

1. 所有到达复活时间的角色复活
2. 如果道具点需要生成道具，生成
3. 发送当前状态给AI
4. 等待接受AI的决策
5. 闪现决策执行
6. 角色、火球、水晶的移动
7. 移动过程中引爆的火球与到达时间的陨石术的伤害判定
8. 角色死亡判定，启动分数机制
9. 陨石术施法，火球术施法判定
10. 水晶的掉落，捡起，水晶进入指定区域引发分数机制（要求水晶不能是无主的）
11. 更新所有时间相关参数

友军伤害配置：调整参数`friendly_fire`开启/关闭友军伤害，关闭友军伤害后友军不会触发子弹的爆炸，友军的攻击手段对友军无伤害

## AI决策

对于自己操纵的每个角色，每帧都可以传递一下参数

1. move : `(x1,y1)` 移动到`x1,y1`处，根据flash参数确定是否由移动变为闪现
2. shoot : `(x1,y1)` 根据当前位置与瞄准的位置确定一个方向，表示向该方向发射火球术
3. meteor : `(x1,y1)` 在该位置释放火球术
4. flash : `True/False` 是否将移动变为闪现

1~3可以传`(-1,-1)`表示不进行操作



