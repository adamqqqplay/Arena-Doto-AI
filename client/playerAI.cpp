#include "playerAI.h"
#include <queue>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <math.h>
#include "log/log.h"
#include <sys/time.h>
#include <map>
using namespace std;

Logic *logic;
Log mylog = Log(LOG_INFO);

//获取毫秒时间戳
int getCurrentTimeStamp()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	return ms;
}

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
	long int startTimeStamp = getCurrentTimeStamp();

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
	pair<int, int> st;
	while (!Q.empty()) //非空队列
	{
		st = Q.front();					   //获取队列中的第一个点
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

	mylog.write(YFL, LOG_INFO, "search point:(%f,%f)", v.x, v.y);
	long int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "	costTime:%ldms", endTimeStamp - startTimeStamp);
	//当遍历完之后，离目标点越近的点dis越小
}

int getCachedPointIndex(Point v, int num)
{
	int pointIndex;
	for (int i = 0; i < 100; i++)
	{
		Point point = pathPoints[i];
		if (point == v) //判断点的路径是否已被缓存
		{
			return i;
		}
	}
	pointIndex = 5 + num;
	pathPoints[pointIndex] = v;
	searchDistance(pathPoints[pointIndex], pathPointsDistance[pointIndex]); //对没有缓存的点搜索
	return pointIndex;
}

void moveTo(int dis1[320][320], int num, Point targetLocation)
{
	Point myPosition = GetUnit(num).position;
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
			tempDis[i] = dis1[tpx][tpy];
		}
		for (int i = 0; i < 4; i++)
		{
			if (tempDis[i] < dis1[px][py] && tempDis[i] == tempDis[(i + 1) % 4])
			{
				move_s(num, targetLocation);
				return;
			}
		}
	}

	int minDistance = 99999;
	int minDirect = 0;
	for (int i = 0; i < 4; i++)
	{
		int direct = (i + base) % 4;
		int dx = dv[direct].x, dy = dv[direct].y;
		int x = px + dx; //(x,y)为单位的周围四个点
		int y = py + dy;
		if (dis1[x][y] < minDistance && dis1[x][y] < dis1[px][py] && dis1[x][y] != -1) //当新点的距离比现在的点距离小时进行相对移动
		{
			minDirect = direct;
			minDistance = dis1[x][y];
		}
	}

	Point dv2[4] = {Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, -1)};
	// mylog.write(YFL, LOG_TRACE, "	minDirect is%d", minDirect);
	// mylog.write(YFL, LOG_TRACE, "	minDirectPoint is%lf,%lf", dv2[minDirect].x, dv2[minDirect].y);
	move_s(num, myPosition + dv2[minDirect]);
}

//使用缓存的自动寻路系统
void move_stupid_new(int num, Point targetLocation)
{
	int pointIndex = getCachedPointIndex(targetLocation, num);
	int dis[320][320], distest[320][320];
	for (int i = 0; i < 320; i++)
	{
		for (int j = 0; j < 320; j++)
		{
			dis[i][j] = pathPointsDistance[pointIndex][i][j];
			distest[i][j] = -1;
		}
	}
	moveTo(dis, num, targetLocation);
	return;
}

//// 新贪婪寻路系统
deque<Point> path[5];
Point pathPointsNew[5];

struct PointBox
{
	double x;
	double y;
	double distance;
	PointBox(){};
	PointBox(double a, double b, double d)
	{
		x = a;
		y = b;
		distance = d;
	}
};

bool comp(const PointBox &a, const PointBox &b)
{
	return a.distance < b.distance;
}

//贪婪寻路 最好不要创建对象
void quickSearchDistance(Point goal, int dis1[320][320], Point myLocation)
{
	long int startTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "quick search point:(%f,%f)", goal.x, goal.y);

	if (isPointValid(goal) == false)
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

	pair<int, int> st;
	vector<PointBox> tempPoint;
	Q.push(mp((int)goal.x, (int)goal.y)); //将目标点放入队列
	dis1[(int)goal.x][(int)goal.y] = 0;   //将目标点的距离设置为0
	Point dv[4] = {Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, -1)};

	int count = 0;
	while (!Q.empty()) //非空队列
	{
		st = Q.front(); //获取队列中的第一个点
		Q.pop();		//弹出该点
		int stx = st.first;
		int sty = st.second;
		int D = dis1[stx][sty]; //大D为此点的距离

		if (dist(Point(stx, sty), myLocation) < 1)
		{
			break;
		}

		for (int direct = 0; direct < 4; direct++)
		{
			int dx = 0, dy = 0;
			int x = dv[direct].x + stx; //(x,y)为该点的周围四个点
			int y = dv[direct].y + sty;
			if (!isWall(x, y) && dis1[x][y] == -1) //当这个周围的点不是墙，并且该点没有被遍历过
			{
				double distance = dist(Point(x, y), myLocation);
				PointBox pb;
				pb.x = x;
				pb.y = y;
				pb.distance = distance;
				tempPoint.push_back(pb);
				dis1[x][y] = D + 1; //那么距离+1
			}
		}
		sort(tempPoint.begin(), tempPoint.end(), comp);
		for (int i = 0; i < tempPoint.size(); i++)
		{
			Q.push(mp(tempPoint[i].x, tempPoint[i].y)); //同时将新点放入队列
		}
		tempPoint.clear();
		count++;
	}

	long int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "	costTime:%ldms,count:%d", endTimeStamp - startTimeStamp, count);
	//当遍历完之后，离目标点越近的点dis越小
}

struct PriorityPoints
{
	Point point;
	double priority;
	PriorityPoints(){};
	~PriorityPoints(){};
	PriorityPoints(Point po, double pri)
	{
		point = po;
		priority = pri;
	}
	friend bool operator<(PriorityPoints a, PriorityPoints b)
	{
		return a.priority > b.priority; //结构体中，priority小的优先级高
	}
};

int heuristic(Point a, Point b)
{
	return abs(a.x - b.x) + abs(a.y - b.y);
}

int pointToIndex(int x, int y)
{
	return 100 * x + y;
}

int pointToIndex(Point p)
{
	int px = floor(p.x);
	int py = floor(p.y);
	return 100 * px + py;
}

void indexToPoint(int index, int result[2])
{
	int y = index % 100;
	int x = index - y;
	result[0] = x;
	result[1] = y;
}

void greedSearch(Point start, Point goal, deque<Point> *path)
{
	while (!(*path).empty())
	{
		(*path).pop_front();
	}
	Point trueGoal = goal;
	start.x = floor(start.x);
	start.y = floor(start.y);
	goal.x = floor(goal.x);
	goal.y = floor(goal.y);
	int startTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "greedSearch start(%lf,%lf),point:(%lf,%lf)", start.x, start.y, goal.x, goal.y);

	priority_queue<PriorityPoints> frontier;
	frontier.push(PriorityPoints(start, 0));

	map<int, Point> came_from;
	map<int, int> visited;

	came_from.insert({pointToIndex(start), Point(-1, -1)});

	Point dv[4] = {Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, -1)};

	int count = 0;
	Point current, next;
	while (!frontier.empty())
	{
		current = frontier.top().point;
		frontier.pop();
		//mylog.write(YFL, LOG_INFO, "current:%f,%f,goal:%f,%f,dist:%f", current.x, current.y, goal.x, goal.y, dist(current, goal));
		if (dist(current, goal) < 1 or count >= 1000)
		{
			break;
		}

		for (int i = 0; i < 4; i++)
		{
			next = current + dv[i];
			bool notInVisited = visited[pointToIndex(next)] == 0;
			bool isPointPass = !isWallNearyBy(next);
			if (notInVisited and isPointPass)
			{
				int priority = dist(goal, next);
				frontier.push(PriorityPoints(next, priority));
				came_from.insert({pointToIndex(next), current});
				visited[pointToIndex(next)] = 1;
			}
		}
		count++;
	}

	int count2 = 0;
	current = goal;
	(*path).push_front(trueGoal);
	Point before;
	while (dist(current, start) > 1 and count2 < count)
	{
		before = came_from[pointToIndex(current)];
		(*path).push_front(current);
		//path = current;
		current = before;
		count2++;
	}

	int endTimeStamp = getCurrentTimeStamp();

	mylog.write(YFL, LOG_INFO, "	costTime:%dms,count:%d,count2:%d", endTimeStamp - startTimeStamp, count, count2);

	return;
}

//近距离快速寻路系统
void move_quick(int num, Point targetLocation)
{
	int dis[320][320];
	quickSearchDistance(targetLocation, dis, GetUnit(num).position);
	moveTo(dis, num, targetLocation);
	return;
}

//测试寻路
void move_greed(int num, Point targetLocation)
{
	Point start = GetUnit(num).position;
	Point goal = targetLocation;
	mylog.write(YFL, LOG_INFO, "move_greed:num:%d,start:%lf,%lf goal:%lf,%lf", num, start.x, start.y, goal.x, goal.y);
	if (start != goal)
	{
		if (goal != pathPointsNew[num] or path[num].empty()) //判断该点是否已求过路径
		{
			pathPointsNew[num] = goal;
			greedSearch(start, goal, &path[num]);
		}

		// int size = path[num].size(), nearByPointIndex;
		// for (int i = 0; i < size; i++)
		// {
		// 	if (dist(start, path[num][i]) <= 0.5)
		// 	{
		// 		nearByPointIndex = i;
		// 		break;
		// 	}
		// }
		// for (int i = 0; i <= nearByPointIndex; i++)
		// {
		// 	if (!path[num].empty())
		// 	{
		// 		path[num].pop_front();
		// 	}
		// }

		Point target = path[num].front();

		if (dist(start, target) <= 0.5)
		{
			path[num].pop_front();
			target = path[num].front();
		}

		move_s(num, target);

		int size1 = path[num].size();
		for (int i = 0; i < size1; i++)
		{
			if (dist(start, path[num][i]) >= CONST::flash_distance and canflash(num))
			{
				flash_s(num, path[num][i]);
				pathPointsNew[num] = Point(-1, -1);
				break;
			}
		}
		mylog.write(YFL, LOG_INFO, "	from %lf,%lf,move to %lf,%lf  path size:%d dist:%lf", start.x, start.y, target.x, target.y, path[num].size(), dist(start, target));
	}
}
////

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

deque<Point> enemyHistoryLocation[5];
void recordHistroyLocation()
{
	for (int i = 0; i < 5; i++)
	{
		Human enemy = GetEnemyUnit(i);
		if (enemy.hp < 0)
		{
			enemyHistoryLocation[i].resize(0);
		}
		else
		{
			if (enemyHistoryLocation[i].size() > 100)
			{
				enemyHistoryLocation[i].pop_back();
			}
			enemyHistoryLocation[i].push_front(enemy.position);
		}
	}
	return;
}

void setDefaultTargetLocation(Point dest[])
{
	dest[0] = logic->crystal[logic->faction].position;				 //我方水晶
	dest[1] = logic->map.bonus_places[0];							 //赏金符点0
	dest[2] = logic->map.bonus_places[1];							 //赏金符点1
	dest[3] = dest[4] = logic->crystal[logic->faction ^ 1].position; //前往敌方水晶
	if (~logic->crystal[logic->faction ^ 1].belong)
	{
		dest[3] = dest[4] = logic->map.target_places[logic->faction]; //将水晶送回家
	}

	for (int i = 0; i < 5; i++)
	{
		Point myPosition = GetUnit(i).position;
		Point birthPosition = logic->map.birth_places[logic->faction][i];
		double distance;
		if (logic->faction == 0)
		{
			distance = 35;
		}
		else
		{
			distance = 25;
		}

		if (dist(myPosition, birthPosition) < distance)
		{
			dest[i] = logic->map.target_places[logic->faction];
		}
	}
}

double getExtrapolatedSpeed(int heroIndex)
{
	if (enemyHistoryLocation[heroIndex].size() < 100)
	{
		mylog.write(YFL, LOG_TRACE, "extrapolatedSpeed[%d] is %lf", heroIndex, CONST::human_velocity * CONST::frames_per_second);
		return CONST::human_velocity * CONST::frames_per_second;
	}
	Human hero = GetEnemyUnit(heroIndex);
	Point nowLocation = hero.position;
	Point historyLocation = enemyHistoryLocation[heroIndex][40];
	double speed = dist(nowLocation, historyLocation) / 40 * CONST::frames_per_second;
	mylog.write(YFL, LOG_TRACE, "extrapolatedSpeed[%d] is %lf", heroIndex, speed);
	return speed;
}

Point getExtrapolatedLocation(int heroIndex, double time)
{
	int frame = time * 20;
	if (frame > 100)
	{
		frame = 100;
	}

	Human hero = GetEnemyUnit(heroIndex);
	Point nowLocation = hero.position;

	if (enemyHistoryLocation[heroIndex].size() < 100)
	{
		mylog.write(YFL, LOG_TRACE, "futureLocation[%d] is (%lf,%lf)", heroIndex, nowLocation.x, nowLocation.y);
		return nowLocation;
	}

	Point historyLocation = enemyHistoryLocation[heroIndex][40];
	mylog.write(YFL, LOG_TRACE, "historyLocation[%d] is (%lf,%lf)", heroIndex, historyLocation.x, historyLocation.y);
	double x = nowLocation.x + (nowLocation.x - historyLocation.x) / 40 * frame;
	double y = nowLocation.y + (nowLocation.y - historyLocation.y) / 40 * frame;
	Point futureLocation = Point(x, y);
	mylog.write(YFL, LOG_TRACE, "futureLocation[%d] is (%lf,%lf)", heroIndex, x, y);
	return futureLocation;
}

bool hasFlash(int heroIndex, int frame)
{
	Human enemy = GetEnemyUnit(heroIndex);
	if (enemy.flash_time > frame)
	{
		return false;
	}
	return true;
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
	if (pathPoints[0] != logic->crystal[logic->faction].position)
	{
		pathPoints[0] = logic->crystal[logic->faction].position;
		searchDistance(pathPoints[0], pathPointsDistance[0]);
	}
	if (pathPoints[3] != logic->crystal[logic->faction ^ 1].position)
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
			if (logic->crystal[logic->faction ^ 1].position == logic->map.crystal_places[logic->faction ^ 1])
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
		Point targetPoint = logic->map.birth_places[logic->faction ^ 1][0]; //敌方出生点
		int targetHeroIndex = 0;
		for (int j = 0; j < 5; j++)
		{
			Human unit = GetEnemyUnit(j);
			if (unit.hp <= 0)
				continue;
			if (dist(unit.position, myPostion) < dist(targetPoint, myPostion))
			{
				targetPoint = unit.position; //寻找最近的敌方单位
				targetHeroIndex = j;
			}
		}
		double D = dist(targetPoint, myPostion) * 0.1;

		Human enemy = GetEnemyUnit(targetHeroIndex);
		Point enemyLocation = enemy.position;

		double targetSpeed = getExtrapolatedSpeed(targetHeroIndex);
		double trackFlyTime = dist(myPostion, enemyLocation) / (CONST::fireball_velocity * CONST::frames_per_second);
		Point shootPoint = getExtrapolatedLocation(targetHeroIndex, trackFlyTime);

		logic->shoot(i, targetPoint); //使用火球

		Point meteorPoint = getExtrapolatedLocation(targetHeroIndex, 2);
		if (dist(meteorPoint, myPostion) <= CONST::meteor_distance && hasFlash(targetHeroIndex, CONST::meteor_delay) == false)
		{
			logic->meteor(i, meteorPoint); //使用陨石
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

		// move_stupid_new(i, finalDest); //先寻路，再闪现
		// if (dist(myPostion, finalDest) > CONST::flash_distance / 2)
		// {
		// 	flash_s(i, finalDest);
		// }
		move_greed(i, finalDest); 
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

	int frame = logic->frame;
	mylog.write(YFL, LOG_INFO, "frame:%d", frame);
	int startTimeStamp = getCurrentTimeStamp();

	if (logic->frame == 1) //第一帧初始化种子
	{
		srand(time(NULL));
		initPoints();
		//initSearch();
	}
	//recheckPoints();

	Point dest[5];

	setDefaultTargetLocation(dest);
	for (int i = 0; i < 5; i++)
	{
		move_s(i, dest[i]);
	}

	recordHistroyLocation();

	attackAndCast();
	getTargetByPath(dest);
	//randomDest(dest);
	avoidAttack(dest);
	executeMove(dest);

	long int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "costTime:%ldms", endTimeStamp - startTimeStamp);
	if (endTimeStamp - startTimeStamp > 50)
	{
		mylog.write(YFL, LOG_WARNING, "long frame detected");
	}
	return;
}