#include "playerAI.h"
#include <queue>
#include <cstring>
#include <algorithm>
#include <ctime>
using namespace std;

Logic *logic;

//根据队伍和英雄编号获取真实ID
int node_translate(int faction, int num)
{
	return num * logic->map.faction_number + faction;
}
//获取我方单位
Human GetUnit(int num)
{
	return logic->humans[node_translate(logic->faction, num)];
}
//获取敌方单位
Human GetEnemyUnit(int num)
{
	return logic->humans[node_translate(logic->faction ^ 1, num)];
}
//获取单位
Human GetUnit(int faction, int num)
{
	return logic->humans[node_translate(faction, num)];
}
//是否为墙
bool isWall(int x, int y)
{
	if (x < 0 || y < 0 || x >= logic->map.width || y >= logic->map.height)
		return true;
	return logic->map.pixels[x][y];
}
//是否为墙
bool isWall(Point v)
{
	return isWall(v.x, v.y);
}

//使指定单位朝目标点移动
void move(int num, Point v)
{
	if (!isWall(v))
		logic->move(num, v);
}
//使用向量进行移动
void move_relative(int num, Point v)
{
	double mod2 = v.x * v.x + v.y * v.y;
	if (mod2 > (CONST::human_velocity - 0.0001) * (CONST::human_velocity - 0.0001))
	{
		mod2 = sqrt(mod2);
		mod2 = (CONST::human_velocity - 0.0001) / mod2;
		v.x *= mod2;
		v.y *= mod2;
	}
	Point pos = GetUnit(num).position;
	v.x += pos.x;
	v.y += pos.y;
	move(num, v);
}
//使用点进行移动
void move_s(int num, Point v)
{
	Point pos = GetUnit(num).position;
	v.x -= pos.x;
	v.y -= pos.y;
	move_relative(num, v);
}

int dis[320][320], inf_dis;
queue<pair<int, int>> Q;
#define mp(a, b) make_pair((a), (b))
//自动寻路系统
void move_stupid(int num, Point v)
{
	memset(dis, 127, sizeof dis);
	memset(&inf_dis, 127, sizeof(int));
	Point pos = GetUnit(num).position;
	Q.push(mp((int)v.x, (int)v.y));
	dis[(int)v.x][(int)v.y] = 0;
	while (!Q.empty()) //非空队列
	{
		pair<int, int> st = Q.front();
		Q.pop();
		int D = dis[st.first][st.second];
		for (int direct = 0; direct < 4; direct++)
		{
			int dx = 0, dy = 0, x, y;
			if (direct == 0)
				dx = -1;
			if (direct == 1)
				dx = 1;
			if (direct == 2)
				dy = -1;
			if (direct == 3)
				dy = 1;
			x = dx + st.first;
			y = dy + st.second;
			if (!isWall(x, y) && dis[x][y] == inf_dis)
			{
				dis[x][y] = D + 1;
				Q.push(mp(x, y));
			}
		}
	}
	//move_s(num,v);
	//return;
	int px = pos.x, py = pos.y;
	int base = rand() % 4;
	for (int i = 0; i < 4; i++)
	{
		int direct = (i + base) % 4;
		int dx = 0, dy = 0, x, y;
		if (direct == 0)
			dx = -1;
		if (direct == 1)
			dx = 1;
		if (direct == 2)
			dy = -1;
		if (direct == 3)
			dy = 1;
		x = px + dx;
		y = py + dy;
		if (dis[x][y] < dis[px][py])
		{
			if (dis[px + dx * 2][py + dy * 2] < dis[x][y])
				dx *= 2, dy *= 2;
			move_relative(num, Point(dx, dy));
			return;
		}
	}
}

//是否允许闪现
bool canflash(int num)
{
	//不处在冷却中，而且不携带水晶
	return GetUnit(num).flash_time <= 0 && logic->crystal[logic->faction ^ 1].belong != node_translate(logic->faction, num);
}
//使指定单位朝目标点闪烁
void flash(int num, Point v)
{
	if (!canflash(num))
		return;
	if (isWall(v))
		return;
	logic->flash(num);
	logic->move(num, v);
}
//使用向量进行闪现
void flash_relative(int num, Point v)
{
	//移动向量平方和
	double mod2 = v.x * v.x + v.y * v.y;
	//移动距离超出最大距离
	if (mod2 > (CONST::flash_distance - 0.0001) * (CONST::flash_distance - 0.0001))
	{
		mod2 = sqrt(mod2);
		mod2 = (CONST::flash_distance - 0.0001) / mod2;
		v.x = v.x * mod2;
		v.y = v.y * mod2;
	}
	Point pos = GetUnit(num).position;
	v.x = v.x + pos.x;
	v.y = v.y + pos.y;
	flash(num, v);
}
//使用点进行闪现
void flash_s(int num, Point v)
{
	Point pos = GetUnit(num).position;
	v.x = v.x - pos.x;
	v.y = v.y - pos.y;
	flash_relative(num, v);
}
//随机浮点数
double RandDouble() { return rand() / (double)RAND_MAX; }
//指定范围的随机浮点数
double RandDouble(double L, double R) { return RandDouble() * (R - L) + L; }

//主思考函数
void playerAI()
{
	logic = Logic::Instance(); //获取实例
	if (logic->frame == 1)	 //第一帧初始化种子
		srand(time(NULL));
	Point dest[5], rush;
	dest[0] = logic->crystal[logic->faction].position;  //我方水晶
	dest[1] = logic->map.bonus_places[0];				//赏金符点0
	dest[2] = logic->map.bonus_places[1];				//赏金符点1
	rush = logic->crystal[logic->faction ^ 1].position; //前往敌方水晶
	if (~logic->crystal[logic->faction ^ 1].belong)
		rush = logic->map.target_places[logic->faction]; //将水晶送回家
	dest[4] = dest[3] = rush;
	for (int i = 0; i < 5; i++)
		move_stupid(i, dest[i]), flash_s(i, dest[i]); //先寻路，再闪现
	for (int i = 0; i < 5; i++)
	{
		Point mypos = GetUnit(i).position;
		Point targ = logic->map.birth_places[logic->faction ^ 1][0]; //敌方出生点
		for (int j = 0; j < 5; j++)
		{
			Human unit = GetEnemyUnit(j);
			if (unit.hp <= 0)
				continue;
			if (dist(unit.position, mypos) < dist(targ, mypos))
				targ = unit.position; //寻找最近的敌方单位
		}
		double D = dist(targ, mypos) * 0.2;
		logic->shoot(i, Point(targ.x + RandDouble(-D, D), targ.y + RandDouble(-D, D)));		 //使用火球
		logic->meteor(i, Point(targ.x + RandDouble(-10, 10), targ.y + RandDouble(-10, 10))); //使用陨石
	}
}