#include "playerAI.h"
#include <queue>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <math.h>
#include "log/log.h"
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
//是否为有效点
bool isPointValid(Point p)
{
	if (p.x < 0 || p.y < 0 || p.x >= logic->map.width || p.y >= logic->map.height)
	{
		return false;
	}
	return true;
}
//是否为墙
bool isWall(Point v)
{
	return isWall(v.x, v.y);
}
bool isWallNearyBy(Point v)
{
	if (isWall(v) == true)
	{
		return true;
	}
	for (int i = 1; i <= 3; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			Point targetPoint = getNearbyPoint(v, deg2rad(45 * j), i);
			if (isWall(targetPoint) == true)
			{
				return true;
			}
		}
	}
	return false;
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

//寻路系统
vector<Point> pathPoints(100, Point(-1, -1));
//vector<vector<vector<int>>> pathPointsDistance(10, vector<vector<int>>(320, vector<int>(320, -1)));
int pathPointsDistance[100][320][320];
#define mp(a, b) make_pair((a), (b))
void searchDistance(Point v, int dis1[320][320])
{
	if (isPointValid(v) == false)
	{
		return;
	}

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

//使用缓存的自动寻路系统
void move_stupid_new(int num, Point v)
{
	Point myPosition = GetUnit(num).position;
	int pointIndex;
	for (int i = 0; i < 100; i++)
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
	Point dv[4] = {Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, -1)};
	int tempDis[4];
	if (isWallNearyBy(myPosition) == false) //当附近没有墙时直线移动
	{
		for (int i = 0; i < 4; i++)
		{
			Point tempPoint = myPosition + dv[i];
			int tpx = tempPoint.x, tpy = tempPoint.y;
			tempDis[i] = dis[tpx][tpy];
		}
		for (int i = 0; i < 4; i++)
		{
			if (tempDis[i] < dis[px][py] && tempDis[i] == tempDis[(i + 1) % 4])
			{
				move_s(num, v);
				return;
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int direct = (i + base) % 4;
		int dx = dv[direct].x, dy = dv[direct].y;
		int x = px + dx; //(x,y)为单位的周围四个点
		int y = py + dy;
		if (dis[x][y] < dis[px][py]) //当新点的距离比现在的点距离小时进行相对移动
		{
			if (dis[px + dx * 2][py + dy * 2] < dis[x][y]) //当现在的点位移2格之后比新点距离小时
				dx *= 2, dy *= 2;
			move_relative(num, Point(dx, dy));
			return;
		}
	}
}

//随机浮点数
double RandDouble() { return rand() / (double)RAND_MAX; }
//指定范围的随机浮点数
double RandDouble(double L, double R) { return RandDouble() * (R - L) + L; }

//是否为敌方玩家
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
		if (isEnemy(metor.from_number) && metor.last_time <= CONST::meteor_delay / 4 && metor.last_time > 0)
		{
			if (dist(metor.position, v) <= CONST::explode_radius + 2)
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
	for (int i = 0; i < 10; i++)
	{
		Point basePoint = Point(v.x, v.y + i);
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
	double randomDeg = RandDouble(0, 360);
	return rotate(v, deg2rad(randomDeg), Point(v.x, v.y + distance));
}

vector<Point> highLevelPoints(5, Point(-1, -1));
vector<double> highLevelTimer(5, -1);
Point fixedCrystalPosition[2], fixedTargetPosition[2];

//对常用点执行搜索
void initSearch()
{
	int count = pathPoints.size();
	for (int i = 0; i < count; i++)
	{
		searchDistance(pathPoints[i], pathPointsDistance[i]);
	}
	return;
}

//初始化常用点
void initPoints()
{
	fixedCrystalPosition[0] = Point(72.5 + 10, 67.5 + 2.5);   //Point(logic->map.crystal_places[0].x + 3, logic->map.crystal_places[0].y + 0);
	fixedCrystalPosition[1] = Point(247.5 - 10, 264.5 - 2.5); //Point(logic->map.crystal_places[1].x - 3, logic->map.crystal_places[1].y + 0);
	fixedTargetPosition[0] = fixedCrystalPosition[0];
	fixedTargetPosition[1] = fixedCrystalPosition[1];
	pathPoints[0] = logic->crystal[logic->faction].position;
	pathPoints[1] = logic->map.bonus_places[0];
	pathPoints[2] = logic->map.bonus_places[1];
	pathPoints[3] = logic->crystal[logic->faction ^ 1].position;
	pathPoints[4] = logic->map.target_places[logic->faction];
	pathPoints[10] = Point(170, 150); //地图中心点
	pathPoints[11] = fixedCrystalPosition[0];
	pathPoints[12] = fixedCrystalPosition[1];
	return;
}

//当常用点的位置发生变化时，重新搜索一次
void recheckPoints()
{
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
	return;
}

//我们是否持有对方水晶
bool areWeGetCrystal()
{
	if (logic->crystal[logic->faction ^ 1].belong == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//通过路径点寻路
void getTargetByPath(Point dest[])
{
	Point myCrystal = logic->map.crystal_places[logic->faction];
	Point enemyCrystal = logic->map.crystal_places[logic->faction ^ 1];
	for (int i = 0; i < 5; i++)
	{
		if (i == 1 || i == 2)
		{
			continue;
		}

		Point myPostion = GetUnit(i).position;
		if (areWeGetCrystal() == false)
		{
			if (equal_Point(logic->crystal[logic->faction ^ 1].position, logic->map.crystal_places[logic->faction ^ 1]))
			{
				if (dist(myPostion, fixedCrystalPosition[logic->faction ^ 1]) > 15)
				{
					dest[i] = fixedCrystalPosition[logic->faction ^ 1]; //优化后的位置
				}
				else
				{
					dest[i] = logic->crystal[logic->faction ^ 1].position;
				}
			}
			else
			{
				dest[i] = logic->crystal[logic->faction ^ 1].position; //前往敌方水晶
			}
		}
		else
		{
			if (dist(myPostion, fixedTargetPosition[logic->faction]) > 15)
			{
				dest[i] = fixedTargetPosition[logic->faction]; //将水晶送回家
			}
			else
			{
				dest[i] = logic->map.target_places[logic->faction]; //将水晶送回家
			}
		}
	}

	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;
		if (i == 1 || i == 2)
		{
			continue;
		}

		bool flag1, flag2;

		if (dist(myCrystal, myPostion) < dist(myCrystal, pathPoints[10]) - 2) //过去
		{
			if (dist(myCrystal, pathPoints[10]) <= dist(myCrystal, dest[i]))
			{
				dest[i] = pathPoints[10];
			}
		}
		else if (dist(enemyCrystal, myPostion) < dist(enemyCrystal, pathPoints[10]) - 2) //回来
		{
			if (dist(myCrystal, pathPoints[10]) >= dist(myCrystal, dest[i]))
			{
				dest[i] = pathPoints[10];
			}
		}
	}
	return;
}

//攻击并施法
void attackAndCast()
{
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
	return;
}

//先判断我即将抵达的点是否安全，如果不安全的话寻找一个更好的点
void avoidAttack(Point dest[])
{
	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;
		if (isPointValid(highLevelPoints[i]) == false)
		{
			if (isLocationFaceExplode(myPostion))
			{
				highLevelPoints[i] = findNearbySafeLocation(myPostion);
				highLevelTimer[i] = logic->frame;
			}
			else
			{
				if (isLocationFaceExplode(dest[i]) && dist(myPostion, dest[i]) <= 10)
				{
					highLevelPoints[i] = findNearbySafeLocation(dest[i]);
					highLevelTimer[i] = logic->frame;
				}
			}
		}
		else
		{
			if (isLocationFaceExplode(highLevelPoints[i]))
			{
				highLevelPoints[i] = findNearbySafeLocation(highLevelPoints[i]);
				highLevelTimer[i] = logic->frame;
			}
			if (logic->frame - 20 >= highLevelTimer[i] || (isLocationFaceExplode(dest[i]) == false && isLocationFaceExplode(myPostion) == false))
			{
				highLevelPoints[i] = Point(-1, -1);
				highLevelTimer[i] = -1;
			}
		}
	}
	return;
}

//执行移动
void executeMove(Point dest[])
{
	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;

		Point finalDest = dest[i];

		if (isPointValid(highLevelPoints[i]))
		{
			finalDest = highLevelPoints[i];
		}

		move_stupid_new(i, finalDest); //先寻路，再闪现
		if (dist(myPostion, finalDest) > CONST::flash_distance / 2)
		{
			flash_s(i, finalDest);
		}
	}
	return;
}

//在抵达目的地后随机走位
void randomDest(Point dest[])
{
	for (int i = 0; i < 3; i++)
	{
		Point myPostion = GetUnit(i).position;
		if (dist(myPostion, dest[i]) < 5)
		{
			highLevelPoints[i] = getNearbyRandomLocation(dest[i], RandDouble(0, 3));
		}
	}
}

//主思考函数
void playerAI()
{
	logic = Logic::Instance(); //获取实例
	if (logic->frame == 1)	 //第一帧初始化种子
	{
		srand(time(NULL));
		initPoints();
		initSearch();
	}
	recheckPoints();

	Point dest[5];
	dest[0] = logic->crystal[logic->faction].position; //我方水晶
	dest[1] = logic->map.bonus_places[0];			   //赏金符点0
	dest[2] = logic->map.bonus_places[1];			   //赏金符点1

	attackAndCast();
	getTargetByPath(dest);
	//randomDest(dest);
	avoidAttack(dest);
	executeMove(dest);
	return;
}