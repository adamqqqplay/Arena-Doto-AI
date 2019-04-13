#include "playerAI.h"
#include <queue>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <math.h>
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
bool isWall(double x, double y)
{
	x = floor(x);
	y = floor(y);
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

//寻路系统
vector<Point> pathPoints(10, Point(-1, -1));
//vector<vector<vector<int>>> pathPointsDistance(10, vector<vector<int>>(320, vector<int>(320, -1)));
int pathPointsDistance[10][320][320];
#define mp(a, b) make_pair((a), (b))
void searchDistance(Point v, int dis1[320][320])
{
	queue<pair<int, int>> Q;
	for (int i = 0; i < 320; i++)
	{
		for (int j = 0; j < 320; j++)
		{
			dis1[i][j] = -1;
		}
	}
	Q.push(mp((int)v.x, (int)v.y)); //将目标点放入队列
	dis1[(int)v.x][(int)v.y] = 0;   //将目标点的距离设置为0
	while (!Q.empty())				//非空队列
	{
		pair<int, int> st = Q.front();	 //获取队列中的第一个点
		Q.pop();						   //弹出该点
		int D = dis1[st.first][st.second]; //大D为此点的距离
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
			x = dx + st.first; //(x,y)为该点的周围四个点
			y = dy + st.second;
			if (!isWall(x, y) && dis1[x][y] == -1) //当这个周围的点不是墙，并且该点没有被遍历过
			{
				dis1[x][y] = D + 1; //那么距离+1
				Q.push(mp(x, y));   //同时将新点放入队列
			}
		}
	}
	//当遍历完之后，离目标点越近的点dis越小
}

//新自动寻路系统
void move_new(int num, Point v)
{
	Point myPosition = GetUnit(num).position;
	int pointIndex;
	for (int i = 0; i < 10; i++)
	{
		Point point = pathPoints[i];
		if (equal_Point(point, v))
		{
			pointIndex = i;
		}
		else
		{
			pointIndex = 5 + num;
			searchDistance(v, pathPointsDistance[pointIndex]);
		}
	}

	double minDistance = 999;
	Point targetPoint;
	Point tempPoint[4];
	double tempDistance[4];
	for (int i = 0; i < 4; i++)
	{
		tempPoint[i] = getNearbyPoint(myPosition, deg2rad(90 * i), 1); //4个方向上的新点
		int x = tempPoint[i].x;
		int y = tempPoint[i].y;
		tempDistance[i] = pathPointsDistance[pointIndex][x][y];
		if (tempDistance[i] < minDistance)
		{
			minDistance = tempDistance[i];
			targetPoint = tempPoint[i];
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (tempDistance[i] == tempDistance[(i + 1) % 4])
		{
			targetPoint = getNearbyPoint(myPosition, deg2rad(45 * (i * 2 + 1)), 1);
		}
	}

	move_s(num, targetPoint);
	return;
}

////
//类似于广度优先搜索的自动寻路系统
void move_stupid(int num, Point v)
{
	int dis[320][320], inf_dis;
	queue<pair<int, int>> Q;
	for (int i = 0; i < 320; i++)
	{
		for (int j = 0; j < 320; j++)
		{
			dis[i][j] = -1;
		}
	}
	inf_dis = -1;
	Point pos = GetUnit(num).position;
	Q.push(mp((int)v.x, (int)v.y)); //将目标点放入队列
	dis[(int)v.x][(int)v.y] = 0;	//将目标点的距离设置为0
	while (!Q.empty())				//非空队列
	{
		pair<int, int> st = Q.front();	//获取队列中的第一个点
		Q.pop();						  //弹出该点
		int D = dis[st.first][st.second]; //大D为此点的距离
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
			x = dx + st.first; //(x,y)为该点的周围四个点
			y = dy + st.second;
			if (!isWall(x, y) && dis[x][y] == inf_dis) //当这个周围的点不是墙，并且该点没有被遍历过
			{
				dis[x][y] = D + 1; //那么距离+1
				Q.push(mp(x, y));  //同时将新点放入队列
			}
		}
	}
	//当遍历完之后，离目标点越近的点dis越小

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
		x = px + dx; //(x,y)为单位的周围四个点
		y = py + dy;
		if (dis[x][y] < dis[px][py]) //当新点的距离比现在的点距离小时进行相对移动
		{
			if (dis[px + dx * 2][py + dy * 2] < dis[x][y]) //当现在的点位移2格之后比新点距离小时
				dx *= 2, dy *= 2;
			move_relative(num, Point(dx, dy));
			return;
		}
	}
}

//使用缓存的自动寻路系统
void move_stupid_new(int num, Point v)
{
	Point myPosition = GetUnit(num).position;
	int pointIndex;
	for (int i = 0; i < 10; i++)
	{
		Point point = pathPoints[i];
		if (equal_Point(point, v)) //判断点的路径是否已被缓存
		{
			pointIndex = i;
		}
		else
		{
			pointIndex = 5 + num;
			if (!equal_Point(pathPoints[pointIndex], v))
			{
				pathPoints[pointIndex] = v;
				searchDistance(pathPoints[pointIndex], pathPointsDistance[pointIndex]); //对没有缓存的点搜索
			}
		}
	}

	int dis[320][320];
	queue<pair<int, int>> Q;
	for (int i = 0; i < 320; i++)
	{
		for (int j = 0; j < 320; j++)
		{
			dis[i][j] = pathPointsDistance[pointIndex][i][j];
		}
	}

	int px = myPosition.x, py = myPosition.y;
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
		x = px + dx; //(x,y)为单位的周围四个点
		y = py + dy;
		if (dis[x][y] < dis[px][py]) //当新点的距离比现在的点距离小时进行相对移动
		{
			if (dis[px + dx * 2][py + dy * 2] < dis[x][y]) //当现在的点位移2格之后比新点距离小时
				dx *= 2, dy *= 2;
			move_relative(num, Point(dx, dy));
			return;
		}
	}
}
////

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

////////////////////////////////////////////MY FUNCTION

bool isEnemy(int num)
{
	if (num % 2 != logic->faction)
	{
		return true;
	}
	return false;
}

//目标区域是否危险
bool isLocationFaceExplode(Point v)
{
	vector<Meteor> metors = logic->meteors;
	//vector<Fireball> fireballs = logic->fireballs;
	for (auto metor : metors)
	{
		if (isEnemy(metor.from_number) && metor.last_time <= CONST::meteor_delay / 1 && metor.last_time > 0)
		{
			if (dist(metor.position, v) <= CONST::explode_radius + 0.5)
			{
				return true;
			}
		}
	}

	return false;
}

//寻找附近的安全位置
Point findNearbySafeLocation(Point v)
{
	int x, y;
	x = v.x;
	y = v.y;

	for (int i = 0; i < 10; i++)
	{
		Point basePoint = Point(x, y + i);
		for (int j = 0; j < 8; j++)
		{
			Point newPoint = rotate(v, deg2rad(45 * j), basePoint);
			if (isLocationFaceExplode(newPoint) == false)
			{
				return newPoint;
			}
		}
	}

	return v;
}

//获取附近的随机位置
Point getNearbyRandomLocation(Point v, double distance)
{
	int x, y;
	x = v.x;
	y = v.y;

	double randomDeg = RandDouble(0, 360);
	Point newPoint = rotate(v, deg2rad(randomDeg), Point(x, y + distance));
	return newPoint;
}
////////////////////////////////////////////END MY FUNCTION

Point highLevelPoints[5] = {Point(-1, -1), Point(-1, -1), Point(-1, -1), Point(-1, -1), Point(-1, -1)};

bool isPointValid(Point p)
{
	if (p.x < 0 || p.y < 0)
	{
		return false;
	}
	return true;
}

//主思考函数
void playerAI()
{
	logic = Logic::Instance(); //获取实例
	if (logic->frame == 1)	 //第一帧初始化种子
	{
		srand(time(NULL));
		//获取常用点的坐标
		pathPoints[0] = logic->crystal[logic->faction].position;
		pathPoints[1] = logic->map.bonus_places[0];
		pathPoints[2] = logic->map.bonus_places[1];
		pathPoints[3] = logic->crystal[logic->faction ^ 1].position;
		pathPoints[4] = logic->map.target_places[logic->faction];
		for (int i = 0; i < 5; i++)
		{
			searchDistance(pathPoints[i], pathPointsDistance[i]); //对常用点执行搜索
		}
	}

	//当常用点的位置发生变化时，重新搜索一次
	if (!equal_Point(pathPoints[0], logic->crystal[logic->faction].position))
	{
		pathPoints[0] = logic->crystal[logic->faction].position;
		searchDistance(pathPoints[0], pathPointsDistance[0]);
	}
	if (!equal_Point(pathPoints[3], logic->crystal[logic->faction ^ 1].position))
	{
		pathPoints[3] = logic->crystal[logic->faction ^ 1].position;
		searchDistance(pathPoints[3], pathPointsDistance[3]);
	}

	Point dest[5], rush;
	dest[0] = logic->crystal[logic->faction].position;  //我方水晶
	dest[1] = logic->map.bonus_places[0];				//赏金符点0
	dest[2] = logic->map.bonus_places[1];				//赏金符点1
	rush = logic->crystal[logic->faction ^ 1].position; //前往敌方水晶
	if (~logic->crystal[logic->faction ^ 1].belong)
	{
		rush = logic->map.target_places[logic->faction]; //将水晶送回家
	}
	dest[3] = dest[4] = rush;

	// //在抵达目的地后随机走位
	// for (int i = 0; i < 3; i++)
	// {
	// 	Point myPostion = GetUnit(i).position;
	// 	if (dist(myPostion, dest[i]) < 5)
	// 	{
	// 		highLevelPoints[i] = getNearbyRandomLocation(dest[i], RandDouble(0, 3));
	// 	}
	// }

	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;

		// //躲避敌方技能
		// if (isPointValid(highLevelPoints[i]))
		// {
		// 	if (isLocationFaceExplode(highLevelPoints[i]))
		// 	{
		// 		highLevelPoints[i] = findNearbySafeLocation(highLevelPoints[i]);
		// 	}
		// 	if (dist(myPostion, highLevelPoints[i]) < 0.1)
		// 	{
		// 		highLevelPoints[i] = Point(-1, -1);
		// 	}
		// }
		// else
		// {
		// 	if (isLocationFaceExplode(myPostion))
		// 	{
		// 		highLevelPoints[i] = findNearbySafeLocation(myPostion);
		// 	}
		// 	else
		// 	{
		// 		if (isLocationFaceExplode(dest[i]))
		// 		{
		// 			highLevelPoints[i] = findNearbySafeLocation(dest[i]);
		// 		}
		// 	}
		// }

		if (isPointValid(highLevelPoints[i]))
		{
			move_stupid_new(i, highLevelPoints[i]); //先寻路，再闪现
			if (dist(myPostion, highLevelPoints[i]) > CONST::flash_distance / 2)
			{
				flash_s(i, highLevelPoints[i]);
			}
		}
		else
		{
			move_stupid_new(i, dest[i]); //先寻路，再闪现
			if (dist(myPostion, dest[i]) > CONST::flash_distance / 2)
			{
				flash_s(i, dest[i]);
			}
		}
	}

	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;
		Point targ = logic->map.birth_places[logic->faction ^ 1][0]; //敌方出生点
		for (int j = 0; j < 5; j++)
		{
			Human unit = GetEnemyUnit(j);
			if (unit.hp <= 0)
				continue;
			if (dist(unit.position, myPostion) < dist(targ, myPostion))
				targ = unit.position; //寻找最近的敌方单位
		}
		double D = dist(targ, myPostion) * 0.1;
		logic->shoot(i, Point(targ.x, targ.y)); //使用火球

		if (dist(Point(targ.x, targ.y), myPostion) <= CONST::meteor_distance - 5)
		{
			logic->meteor(i, Point(targ.x + RandDouble(-1, 1), targ.y + RandDouble(-1, 1))); //使用陨石
		}
	}
}