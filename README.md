# Arena-Doto-AI

为参加清华大学第23届智能体大赛Doto游戏而编写的AI



# 开发环境配置

以windows平台为例

1.克隆本项目至本地

2.安装Arena平台和游戏

(1)前往Arena官网下载对应版本的游戏平台至/Arena-Doto-AI

(2)将Areana平台解压后移动至/Arena-Doto-AI/arena9

(3)运行Arena平台，即/Arena-Doto-AI/arena9/arena9.exe

(4)更新完平台后注册登录，或者使用公共账号

(5)在平台主界面中点击下载游戏，下载完后/Arena-Doto-AI/arena9/Game目录就会出现

3.安装tdm64-gcc编译工具

参见https://arena.net9.org/downloads/games/Doto/compile.pdf

4.安装python3，并安装numpy，pygame模块

5.在/Arena-Doto-AI/client/目录下运行make指令，则会编译生成main.out文件

6.将main.out文件复制到/Arena-Doto-AI/server/目录下，并重命名为main0.exe

7.运行游戏 运行/Arena-Doto-AI/server/目录下的server.py，程序将自动让main0.exe和官方示例mainbase.exe进行对抗。

(5-7).运行/Arena-Doto-AI/client/runbuild.cmd，则会自动开始编译运行的过程

8.播放录像 运行/Arena-Doto-AI/server/目录下的GamePlayerInUnity.py开启游戏播放录像，或者运行GamePlayer.py开启pygame小游戏播放。



如遇到其它问题可以参考官方SDK文档/Arena-Doto-AI-master/docs/sdk

# 参考链接

比赛官网
https://arena.net9.org/downloads

清华AI的华山论剑现已开启，不服就来战！王小川楼天城唐文斌都曾搅动风云
https://mp.weixin.qq.com/s/G6vrsOSXPK5DrCX06th1WA

智能体大赛 | 智能体规则抢先看
https://mp.weixin.qq.com/s/b0omaZ46wJNK4snPVXhpnA

协作开发规范
https://zhuanlan.zhihu.com/p/61145348
https://ruby-china.org/topics/15737