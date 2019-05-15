#include "playerAI.h"
#include <queue>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <math.h>
#include "log/log.h"
#include <sys/time.h>
#include <map>
#include "astar/astar.h"
#include "astar/blockallocator.h"
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
bool isWallNearBy(Point v)
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

int getTeam()
{
	return logic->faction;
}

int getEnemyTeam()
{
	return logic->faction ^ 1;
}

Point getOurCrystalDefaultLocation()
{
	return logic->map.crystal_places[getTeam()];
}

Point getEnemyCrystalDefaultLocation()
{
	return logic->map.crystal_places[getEnemyTeam()];
}

Point getOurCrystalLocation()
{
	return logic->crystal[getTeam()].position;
}

Point getEnemyCrystalLocation()
{
	return logic->crystal[getEnemyTeam()].position;
}

int getFrame()
{
	return logic->frame;
}

//曼哈顿距离评估
int heuristic(Point a, Point b)
{
	return abs(a.x - b.x) + abs(a.y - b.y);
}

int pointToIndex(int x, int y)
{
	return 1000 * x + y;
}

int pointToIndex(Point p)
{
	int px = floor(p.x);
	int py = floor(p.y);
	return 1000 * px + py;
}

void indexToPoint(int index, int result[2])
{
	int y = index % 1000;
	int x = index - y;
	result[0] = x;
	result[1] = y;
}

//是否为敌方玩家
bool isEnemy(int num)
{
	if (num % 2 != logic->faction)
	{
		return true;
	}
	return false;
}

int getTheDistanceToWall(Point location, Point vector)
{
	double locationX = location.x;
	double locationY = location.y;
	double deltaX = vector.x;
	double deltaY = vector.y;
	for (int i = 0; i <= 30; i++)
	{
		if (isWall(locationX, locationY))
		{
			return i;
		}
		locationX += deltaX;
		locationY += deltaY;
	}
	return 30;
}

Line getFireBallLine(Fireball fireball)
{
	Point futureFirePosition = getNearbyPoint(fireball.position, fireball.rotation, CONST::fireball_velocity * 10);
	return makeLine(fireball.position, futureFirePosition);
}

Lineseg getFireBallLineseg(Fireball fireball)
{
	int length = CONST::fireball_velocity * 10;
	Point location = fireball.position;
	Point deltaVector = getNearbyVector(fireball.rotation, 1);
	int realLength = getTheDistanceToWall(location, deltaVector); //火球撞墙后弹道消失
	Point futureFirePosition = getNearbyPoint(fireball.position, fireball.rotation, realLength);
	return Lineseg(fireball.position, futureFirePosition);
}

// double getPointRisk(Point point)
// {
// 	double risk = 0;
// 	vector<Meteor> metors = logic->meteors;
// 	vector<Fireball> fireballs = logic->fireballs;
// 	for (auto metor : metors)
// 	{
// 		if (isEnemy(metor.from_number) && metor.last_time <= CONST::meteor_delay / 2 && metor.last_time > 0)
// 		{
// 			if (dist(metor.position, point) <= CONST::explode_radius + 1)
// 			{
// 				risk += 100;
// 			}
// 		}
// 	}

// 	Point np;
// 	for (auto fireball : fireballs)
// 	{
// 		if (isEnemy(fireball.from_number))
// 		{
// 			double distance = dist(point, fireball.position);
// 			if (distance <= CONST::fireball_velocity * 10)
// 			{
// 				Lineseg line = getFireBallLineseg(fireball);
// 				double straightDistance = ptoLinesegdist(point, line, np);
// 				//double verticalDistance = poinToLinetDistance(point, line);
// 				if (straightDistance <= CONST::fireball_radius + 1)
// 				{
// 					risk += 5;
// 				}
// 			}
// 		}
// 	}
// 	return risk;
// }

Human getTheMostNearbyEnemy(Point myLocation)
{
	double minDistance = 999;
	double distance;
	Human minHuman, current;
	for (int i = 0; i < 5; i++)
	{
		current = GetEnemyUnit(i);
		distance = dist(current.position, myLocation);
		if (distance < minDistance)
		{
			minDistance = distance;
			minHuman = current;
		}
	}
	return minHuman;
}

Fireball getTheMostRecentFireball(Point myLocation)
{
	vector<Fireball> fireballs = logic->fireballs;
	double minDistance = 999;
	double distance;
	Fireball minFireball = Fireball(-1, -1, -1, -1);
	for (auto fireball : fireballs)
	{
		if (isEnemy(fireball.from_number))
		{
			Lineseg line = getFireBallLineseg(fireball);
			Point np;
			double straightDistance = ptoLinesegdist(myLocation, line, np);
			if (straightDistance <= CONST::fireball_radius + 1)
			{
				distance = dist(fireball.position, myLocation);
				if (distance < minDistance)
				{
					minDistance = distance;
					minFireball = fireball;
				}
			}
		}
	}
	return minFireball;
}

Line getVerticalLine(Line line, Point point)
{
	Line newLine(line.b, -line.a, line.a * point.y - line.b * point.x);
}

Point getAvoidVector(Point myLocation)
{
	Point baseVector = Point(0, 0);

	vector<Fireball> fireballs = logic->fireballs;
	double minDistance = 999;
	double distance;
	Fireball minFireball = Fireball(-1, -1, -1, -1);
	for (auto fireball : fireballs)
	{
		if (isEnemy(fireball.from_number))
		{
			Lineseg line = getFireBallLineseg(fireball);
			Point np;
			double straightDistance = ptoLinesegdist(myLocation, line, np);
			if (straightDistance <= CONST::fireball_radius + 0.5)
			{
				Point vector1 = getNearbyPoint(fireball.position, fireball.rotation + deg2rad(90), 1) - fireball.position;
				Point vector2 = getNearbyPoint(fireball.position, fireball.rotation - deg2rad(90), 1) - fireball.position;
				int distanceToWall1 = getTheDistanceToWall(myLocation, vector1);
				int distanceToWall2 = getTheDistanceToWall(myLocation, vector2);
				Point toCenter = fireball.position - myLocation;
				if (vector1 * toCenter < 0 and distanceToWall1 > 3)
				{
					baseVector = baseVector + vector1;
				}
				else if (vector2 * toCenter <= 0 and distanceToWall2 > 3)
				{
					baseVector = baseVector + vector2;
				}
				else if (vector1 * toCenter < 0)
				{
					baseVector = baseVector + vector1;
				}
				else
				{
					baseVector = baseVector + vector2;
				}
			}
		}
	}
	return baseVector;
}

map<int, int> riskMap;
void generateRiskMap()
{
	int startTimeStamp = getCurrentTimeStamp();
	riskMap.clear();
	double risk = 0;
	vector<Fireball> fireballs = logic->fireballs;
	for (auto fireball : fireballs)
	{
		map<int, int> passedMap;
		if (isEnemy(fireball.from_number))
		{
			Lineseg line = getFireBallLineseg(fireball);
			Point delta = (line.e - line.s) / dist(line.e, line.s);
			Point current = line.s;
			int distance = dist(line.s, line.e);
			for (int k = 0; k < distance; k++)
			{
				int fireballX = floor(current.x);
				int fireballY = floor(current.y);
				passedMap[pointToIndex(fireballX, fireballY)] = 1;
				for (int i = -3; i <= 3; i++)
				{
					for (int j = -3; j <= 3; j++)
					{
						passedMap[pointToIndex(fireballX + i, fireballY + j)] = 1;
					}
				}
				current = current + delta;
			}
		}
		for (auto pointIndex : passedMap)
		{
			if (pointIndex.second == 1)
			{
				riskMap[pointIndex.first] += 5;
			}
		}
	}
	vector<Meteor> metors = logic->meteors;
	for (auto metor : metors)
	{
		if (isEnemy(metor.from_number) && metor.last_time <= CONST::meteor_delay / 2 && metor.last_time > 0)
		{
			int metorX = metor.position.x;
			int metorY = metor.position.y;
			riskMap[pointToIndex(metor.position)] += 100;
			for (int i = -3; i <= 3; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					riskMap[pointToIndex(metorX + i, metorY + j)] += 100;
				}
			}
		}
	}

	int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "[generateRiskMap] costTime:%dms,map size:%d", endTimeStamp - startTimeStamp, riskMap.size());
	return;
}

int getRiskMapValue(Point p)
{
	return riskMap[pointToIndex(p)];
}

//寻找附近的安全位置
Point findNearbyNoRiskLocation(Point v)
{
	for (int i = 0; i < 10; i++)
	{
		Point basePoint = Point(v.x, v.y + i);
		for (int j = 0; j < 8; j++)
		{
			Point newPoint = rotate(v, deg2rad(45 * j), basePoint);
			if (getRiskMapValue(newPoint) == 0 and isWall(newPoint) == false)
			{
				return newPoint;
			}
		}
	}
	return v;
}

//寻找附近的可移动点
Point findNearbyMovableLocation(Point v)
{
	for (int i = 0; i < 10; i++)
	{
		Point basePoint = Point(v.x, v.y + i);
		for (int j = 0; j < 8; j++)
		{
			Point newPoint = rotate(v, deg2rad(45 * j), basePoint);
			if (isWall(newPoint) == false)
			{
				return newPoint;
			}
		}
	}
	return v;
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

//附近是否有障碍物
vector<Point> dv8 = {Point(0, 0), Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, -1), Point(1, 1), Point(-1, 1), Point(-1, -1), Point(1, -1)};
bool isWallNearbyNew(double x, double y, double weight)
{
	int size = dv8.size();
	for (int j = 0; j < size; j++)
	{
		for (int i = 0; i <= weight; i++)
		{
			if (isWall(x + dv8[j].x * i, y + dv8[j].y * i))
			{
				return true;
			}
		}
	}
	return false;
}

//两点间是否存在直接路径
bool isLineWalkable(Point start, Point end)
{
	if (isWall(start) or isWall(end))
	{
		return false;
	}

	Line line = makeLine(start, end);
	double lineAlpha = alpha(line);

	int distance = dist(start, end);
	Point stepVec = (end - start) / dist(start, end);

	double stepX = stepVec.x;
	double stepY = stepVec.y;
	double currentX = start.x;
	double currentY = start.y;

	for (int i = 0; i < distance; i++)
	{
		if (isWallNearbyNew(currentX, currentY, 2))
		{
			return false;
		}
		currentX = currentX + stepX;
		currentY = currentY + stepY;
	}
	return true;
}

//floyd路径平滑算法
void Floyd(deque<Point> *path, bool fullOptimize)
{
	int startTimeStamp = getCurrentTimeStamp();
	if (path->empty() or path->size() >= 500)
	{
		return;
	}

	int len = path->size();
	//去掉同一条线上的点。
	if (len > 2)
	{
		Line vector = makeLine(path->at(len - 1), path->at(len - 2));
		Line tempvector;
		for (int i = len - 3; i >= 0; i--)
		{
			tempvector = makeLine(path->at(i + 1), path->at(i));
			if (equalLine(vector, tempvector))
			{
				path->erase(path->begin() + i + 1);
			}
			else
			{
				vector = tempvector;
			}
		}
	}
	//去掉无用拐点
	if (fullOptimize)
	{
		len = path->size();
	}
	else
	{
		int len2 = path->size();
		len = min(len2, 10);
	}

	for (int i = len - 1; i >= 0; i--)
	{
		for (int j = 0; j <= i - 2; j++)
		{
			if (isLineWalkable(path->at(i), path->at(j)))
			{
				for (int k = i - 1; k > j; k--)
				{
					path->erase(path->begin() + k);
				}
				i = j;
				//len = path->size();
				break;
			}
		}
	}
	int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "[Floyd] costTime:%dms", endTimeStamp - startTimeStamp);
	return;
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

	long int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "(searchDistance) point:(%f,%f) costTime:%ldms", v.x, v.y, endTimeStamp - startTimeStamp);
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
	if (isWallNearBy(myPosition) == false) //当附近没有墙时直线移动
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

//测试A*搜索
bool astarSearch(Point start, Point goal, deque<Point> *path)
{
	int startTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "astarSearch start(%lf,%lf),point:(%lf,%lf)", start.x, start.y, goal.x, goal.y);

	while (!(*path).empty())
	{
		(*path).pop_front();
	}

	// 搜索参数
	AStar::Params param;
	param.width = 360;
	param.height = 360;
	param.corner = true;
	param.start = AStar::Vec2(floor(start.x), floor(start.y));
	param.end = AStar::Vec2(floor(goal.x), floor(goal.y));
	param.can_pass = [&](const AStar::Vec2 &pos) -> bool {
		return isWall(pos.x, pos.y) == false;
	};

	// 执行搜索
	BlockAllocator allocator;
	AStar algorithm(&allocator);
	auto path1 = algorithm.find(param);
	int size = path1.size();

	for (int i = 0; i < size; i++)
	{
		AStar::Vec2 point = path1[i];
		path->push_back(Point(point.x, point.y));
	}
	path->push_back(goal);
	int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "	astar costTime:%dms,count:%d", endTimeStamp - startTimeStamp, path->size());

	return true;
}

//优先队列中的点
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

//贪婪优先寻路算法
bool greedSearch(Point start, Point goal, deque<Point> *path)
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

	Point dv[8] = {Point(1, 0), Point(0, 1), Point(-1, 0), Point(0, -1), Point(1, 1), Point(-1, 1), Point(-1, -1), Point(1, -1)};

	int count = 0;
	Point current, next;
	double minDistanceTotarget = 999;
	int warningCount = 0;
	while (!frontier.empty())
	{
		current = frontier.top().point;
		frontier.pop();
		//mylog.write(YFL, LOG_INFO, "current:%f,%f,goal:%f,%f,dist:%f", current.x, current.y, goal.x, goal.y, dist(current, goal));

		double currentDistanceToTarget = dist(current, goal);
		if (currentDistanceToTarget < minDistanceTotarget)
		{
			minDistanceTotarget = currentDistanceToTarget;
		}
		else
		{
			if (currentDistanceToTarget < 5 and currentDistanceToTarget > minDistanceTotarget)
			{
				warningCount++;
			}
		}

		if (currentDistanceToTarget <= 1 or count >= 600 or warningCount >= 3)
		{
			if (count >= 600)
			{
				mylog._warn("	search failed for point (%lf,%lf)", trueGoal.x, trueGoal.y);
			}
			if (warningCount >= 3)
			{
				mylog._warn("	get a Alternative point for (%lf,%lf), minDistanceTotarget:%lf , currentDistanceToTarget:%lf", trueGoal.x, trueGoal.y, minDistanceTotarget, currentDistanceToTarget);
			}
			break;
		}

		for (int i = 0; i < 8; i++)
		{
			next = current + dv[i];
			bool notInVisited = visited[pointToIndex(next)] == 0;
			bool isPointPass = !isWallNearbyNew(next.x, next.y, 1);
			int riskValue = getRiskMapValue(next);
			if (notInVisited and isPointPass and riskValue < 100)
			{
				int priority = dist(goal, next); // + riskValue;
				frontier.push(PriorityPoints(next, priority));
				came_from.insert({pointToIndex(next), current});
				visited[pointToIndex(next)] = 1;
			}
		}
		count++;
	}

	int count2 = 0;
	current = goal;
	if (warningCount < 3)
	{
		(*path).push_front(trueGoal);
	}
	Point before;
	while (dist(current, start) > 1 and count2 < count)
	{
		before = came_from[pointToIndex(current)];
		(*path).push_front(current);
		//path = current;
		current = before;
		count2++;
	}

	//搜索失败寻找附近的点
	if (count == 1 or count2 == 1)
	{
		int random = rand() % 4;
		(*path).push_front(start + dv[random]);
	}

	int endTimeStamp = getCurrentTimeStamp();

	mylog.write(YFL, LOG_INFO, "	costTime:%dms,count:%d,count2:%d", endTimeStamp - startTimeStamp, count, count2);

	if (count < 1000)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int lastSearchFrame[5];
deque<Point> oldPath[5];
//测试寻路
void move_greed(int num, Point targetLocation)
{
	int startTimeStamp = getCurrentTimeStamp();
	Point start = GetUnit(num).position;
	Point goal = targetLocation;
	mylog.write(YFL, LOG_INFO, "move_greed:num:%d,start:%lf,%lf goal:%lf,%lf", num, start.x, start.y, goal.x, goal.y);
	if (start != goal)
	{
		int currentPointRisk = getRiskMapValue(start);
		if (path[num].empty() or goal != pathPointsNew[num] or getFrame() - lastSearchFrame[num] >= 10 or currentPointRisk != 0) //判断该点是否已求过路径
		{
			pathPointsNew[num] = goal;
			bool searchSuccess = greedSearch(start, goal, &path[num]);
			if (searchSuccess == true)
			{
				oldPath[num].assign(path[num].begin(), path[num].end());
				lastSearchFrame[num] = getFrame();
				Floyd(&path[num], true);
			}
		}

		start = GetUnit(num).position;
		Point target = path[num].front();

		if (dist(start, target) <= 1 and path[num].size() > 1)
		{
			path[num].pop_front();
			target = path[num].front();
		}
		mylog._info("		//next path:%lf,%lf", target.x, target.y);

		double dived = dist(start, target);
		if (dived == 0)
		{
			dived = dist(start, goal);
		}
		if (dived == 0)
		{
			target = Point(0, 0);
		}
		else
		{
			target = (target - start) / dived;
		}

		Point fixVector = getAvoidVector(start);
		Point tempdebug = start + target + fixVector;
		mylog._info("		//fixed path:%lf,%lf	start:%lf,%lf	target:%lf,%lf	fixVector:%lf,%lf", tempdebug.x, tempdebug.y, start.x, start.y, target.x, target.y, fixVector.x, fixVector.y);
		target = start + target + fixVector;

		move_s(num, target);

		if (canflash(num))
		{
			int size1 = oldPath[num].size();
			Point flashTarget = Point(-1, -1);
			for (int i = 0; i < size1; i++)
			{
				int distance = dist(start, oldPath[num][i]);
				if (distance >= CONST::flash_distance and distance <= CONST::flash_distance + 2 and !isWall(oldPath[num][i]))
				{
					flashTarget = oldPath[num][i];
					pathPointsNew[num] = Point(-1, -1);
					break;
				}
			}
			if (dist(start, targetLocation) <= CONST::flash_distance and !isWall(targetLocation))
			{
				flashTarget = targetLocation;
				pathPointsNew[num] = Point(-1, -1);
			}

			//寻找替代闪现点
			if (isPointValid(flashTarget))
			{
				if (getRiskMapValue(flashTarget) != 0)
				{
					Point newFlashPoint;
					for (int i = 1; i <= 3; i++)
					{
						newFlashPoint = rotate(start, deg2rad(5 * i), flashTarget);
						if (getRiskMapValue(newFlashPoint) == 0)
						{
							flashTarget = newFlashPoint;
							break;
						}
					}
					for (int i = 1; i <= 3; i++)
					{
						newFlashPoint = rotate(start, deg2rad(-5 * i), flashTarget);
						if (getRiskMapValue(newFlashPoint) == 0)
						{
							flashTarget = newFlashPoint;
							break;
						}
					}
				}
			}

			if (isPointValid(flashTarget))
			{
				flash_s(num, flashTarget);
			}
		}

		mylog.write(YFL, LOG_INFO, "	from %lf,%lf,move to %lf,%lf  path size:%d dist:%lf", start.x, start.y, target.x, target.y, path[num].size(), dist(start, target));
		int endTimeStamp = getCurrentTimeStamp();
		mylog.write(YFL, LOG_INFO, "(move_greed) costTime:%dms", endTimeStamp - startTimeStamp);
	}
}
////

//随机浮点数
double RandDouble() { return rand() / (double)RAND_MAX; }
//指定范围的随机浮点数
double RandDouble(double L, double R) { return RandDouble() * (R - L) + L; }

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
			if (isLocationFaceExplode(newPoint) == false and isWall(newPoint) == false)
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

int startDirection[2];
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

	for (int i = 1; i <= 2; i++)
	{
		Point myLocation = GetUnit(i).position;
		Human enemy = getTheMostNearbyEnemy(myLocation);
		vector<Point> deltaVector = {Point(0, 0), Point(-5, -5), Point(5, -5)};

		if (dist(myLocation, enemy.position) <= 4 and dist(myLocation, dest[i]) <= 5 and enemy.hp > 0)
		{
			dest[i] = enemy.position + Point(-2, -2);
		}
		else if (dist(myLocation, dest[i]) <= 20)
		{
			Point tempPoint;
			tempPoint.x = deltaVector[startDirection[i]].x;
			tempPoint.y = deltaVector[startDirection[i]].y * pow(-1, getTeam());
			if (dist(myLocation, dest[i] + tempPoint) <= 1)
			{
				startDirection[i] = (startDirection[i] + 1) % deltaVector.size();
			}
			dest[i] = dest[i] + tempPoint;
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

//获取敌方水晶所属单位
int getEnemyCrystalBelonging()
{
	for (int i = 0; i < 5; i++)
	{
		if (logic->crystal[logic->faction ^ 1].belong == node_translate(logic->faction, i))
		{
			return i;
		}
	}
	return -1;
}

//获取离该点最近的友方单位
int getNearestUnitToCrystal()
{
	Point point = logic->map.crystal_places[logic->faction ^ 1];
	int unitIndex = 0;
	double minDistance = 999;
	for (int i = 0; i < 5; i++)
	{
		if (i == 1 or i == 2 or GetUnit(i).hp <= 0)
		{
			continue;
		}
		Point myPostion = GetUnit(i).position;
		double distance = dist(myPostion, point);
		if (distance < minDistance)
		{
			minDistance = distance;
			unitIndex = i;
		}
	}
	return unitIndex;
}

queue<Point> movePathQueue[5];
bool pastAttackState; //0==进攻 1==返回
void setMovePathQueue(queue<Point> *Q)
{
	vector<Point> crystalAttackPath = {getOurCrystalDefaultLocation(), fixedTargetPosition[getTeam()], pathPoints[10], fixedTargetPosition[getEnemyTeam()], getEnemyCrystalDefaultLocation()};

	if (areWeGetCrystal())
	{
		reverse(crystalAttackPath.begin(), crystalAttackPath.end());
	}

	int size = crystalAttackPath.size();
	for (int i = 0; i < size; i++)
	{
		Q->push(crystalAttackPath[i]);
	}
}

int secondLeaderIndex = -1;
//通过路径点寻路
void getTargetByPath(Point dest[])
{
	Point myCrystal = getOurCrystalDefaultLocation();
	Point enemyCrystal = getEnemyCrystalDefaultLocation();
	// for (int i = 0; i < 5; i++)
	// {
	// 	Point myLocation = GetUnit(i).position;

	// 	if (i == 1 || i == 2)
	// 	{
	// 		continue;
	// 	}

	// 	if (pastAttackState != areWeGetCrystal() or movePathQueue[i].empty())
	// 	{
	// 		setMovePathQueue(&movePathQueue[i]);
	// 		pastAttackState = areWeGetCrystal();
	// 	}

	// 	if (dist(myLocation, movePathQueue[i].front()) < 2)
	// 	{
	// 		movePathQueue[i].pop();
	// 	}
	// 	dest[i] = movePathQueue[i].front();
	// }

	for (int i = 0; i < 5; i++)
	{
		if (i == 1 || i == 2)
		{
			continue;
		}

		Point myPostion = GetUnit(i).position;
		int myTeam = getTeam();
		int enemyTeam = getEnemyTeam();
		Point enemyCrystalLocation = getEnemyCrystalLocation();
		Point enemyCrystalDefaultLocation = getEnemyCrystalDefaultLocation();
		Point myCrystalDefaultLocation = getOurCrystalDefaultLocation();
		if (areWeGetCrystal() == false)
		{
			if (enemyCrystalLocation == enemyCrystalDefaultLocation and dist(myPostion, fixedCrystalPosition[enemyTeam]) > 15)
			{

				dest[i] = fixedCrystalPosition[enemyTeam]; //优化后的位置
			}
			else
			{
				dest[i] = enemyCrystalLocation; //前往敌方水晶
			}
		}
		else
		{
			if (dist(myPostion, fixedTargetPosition[myTeam]) > 15)
			{
				dest[i] = fixedTargetPosition[myTeam]; //将水晶送回家
			}
			else
			{
				dest[i] = myCrystalDefaultLocation; //将水晶送回家
			}
		}
	}

	int enemyCrystalBelonging = getEnemyCrystalBelonging();
	int nearestUnitToCrystal = getNearestUnitToCrystal();
	int headUnitIndex = 0;
	if (enemyCrystalBelonging != -1)
	{
		headUnitIndex = enemyCrystalBelonging;
	}
	else
	{
		headUnitIndex = nearestUnitToCrystal;
	}

	deque<int> unitQueue;
	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;
		if (i == 1 or i == 2 or i == headUnitIndex)
		{
			continue;
		}
		unitQueue.push_front(i);
	}

	Point fixVector[2];
	Point basePoint;
	int angle = 0;
	if (areWeGetCrystal() == false)
	{
		basePoint = GetUnit(headUnitIndex).position;
		angle = 180;
	}
	else
	{
		basePoint = getEnemyCrystalLocation();
	}

	fixVector[0] = getNearbyVector(deg2rad(90 + getTeam() * 180 + angle), 6);
	fixVector[1] = getNearbyVector(deg2rad(-90 + getTeam() * 180 + angle), 6);
	int size = unitQueue.size();
	for (int i = 0; i < size; i++)
	{
		int unitIndex = unitQueue[i];
		Human unit = GetUnit(unitIndex);
		//unitQueue.pop();
		Point newPoint = basePoint + fixVector[i];
		if (isWall(newPoint) == false and (getRiskMapValue(newPoint) == 0) or dist(newPoint, unit.position) >= 20)
		{
			dest[unitIndex] = newPoint;
		}
	}

	bool secondBackPeriod = dist(getEnemyCrystalLocation(), getOurCrystalDefaultLocation()) < dist(pathPoints[10], getOurCrystalDefaultLocation()) - 20;
	if (secondBackPeriod and areWeGetCrystal())
	{
		int size = unitQueue.size();
		for (int i = 0; i < size; i++)
		{
			int unitIndex = unitQueue[i];
			dest[unitIndex] = getEnemyCrystalDefaultLocation();
		}
	}

	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;
		if (i == 1 || i == 2)
		{
			continue;
		}

		if (dist(myCrystal, myPostion) < dist(myCrystal, pathPoints[10]) - 5) //过去
		{
			if (dist(myCrystal, pathPoints[10]) <= dist(myCrystal, dest[i]))
			{
				dest[i] = pathPoints[10];
			}
		}
		else if (dist(enemyCrystal, myPostion) < dist(enemyCrystal, pathPoints[10]) - 5) //回来
		{
			if (dist(myCrystal, pathPoints[10]) >= dist(myCrystal, dest[i]))
			{
				dest[i] = pathPoints[10];
			}
		}
	}

	vector<Point> topPathPoints(10);
	topPathPoints[0] = Point(104, 86);
	topPathPoints[1] = Point(40, 86);
	topPathPoints[2] = Point(122, 25);
	topPathPoints[3] = Point(104, 86);
	topPathPoints[4] = Point(104, 86);
	topPathPoints[5] = Point(221, 241);
	topPathPoints[6] = Point(195, 293);
	topPathPoints[7] = Point(288, 233);
	topPathPoints[8] = Point(221, 241);
	topPathPoints[9] = Point(221, 241);
	for (int i = 0; i < 5; i++)
	{
		Point myPosition = GetUnit(i).position;
		Point birthPosition = logic->map.birth_places[logic->faction][i];
		double distance;
		if (logic->faction == 0)
		{
			distance = 30;
		}
		else
		{
			distance = 30;
		}

		if (dist(myPosition, birthPosition) < distance)
		{
			dest[i] = fixedCrystalPosition[logic->faction];
		}
		else
		{
			if (i == 1 or i == 2)
			{
				Point tempPathPoint = topPathPoints[getTeam() * 5 + i];
				if (dist(myPosition, birthPosition) < dist(tempPathPoint, birthPosition) - 5)
				{
					dest[i] = tempPathPoint;
				}else if(getTeam()==1 and i==1)
				{
					Point tempPathPoint2 = Point(70,275);
					if(dist(myPosition, birthPosition) < dist(tempPathPoint2, birthPosition) - 5)
					{
						dest[i] = tempPathPoint2;
					}
				}
			}
		}
	}
	return;
}

int getMinHpHeroIndex(Point myPostion)
{
	int minHp = 100;
	int minHpHeroIndex = -1;
	for (int j = 0; j < 5; j++)
	{
		Human unit = GetEnemyUnit(j);
		if (unit.hp <= 0)
			continue;
		double distance = dist(unit.position, myPostion);
		if (distance <= 15 and isLineWalkable(myPostion, unit.position) and unit.hp <= minHp)
		{
			minHp = unit.hp;
			minHpHeroIndex = j;
		}
	}
	return minHpHeroIndex;
}

//攻击并施法
void attackAndCast()
{
	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;
		Point targetPoint = logic->map.birth_places[logic->faction ^ 1][0]; //敌方出生点
		double minDistance = 999;
		int targetHeroIndex = 0;
		for (int j = 0; j < 5; j++)
		{
			Human unit = GetEnemyUnit(j);
			if (unit.hp <= 0)
				continue;
			double distance = dist(unit.position, myPostion);
			if (distance < minDistance)
			{
				minDistance = distance;
				targetHeroIndex = j;
			}
		}

		Human enemy = GetEnemyUnit(targetHeroIndex);
		Point enemyLocation = enemy.position;

		Point shootPoint;
		int minHpHeroIndex = getMinHpHeroIndex(myPostion);
		if (minHpHeroIndex != -1)
		{
			enemy = GetEnemyUnit(minHpHeroIndex);
			enemyLocation = enemy.position;
			shootPoint = enemyLocation;
		}
		else
		{
			double targetSpeed = getExtrapolatedSpeed(targetHeroIndex);
			double trackFlyTime = min(dist(myPostion, enemyLocation) / (CONST::fireball_velocity * CONST::frames_per_second), 2.0);
			shootPoint = getExtrapolatedLocation(targetHeroIndex, trackFlyTime);
		}
		logic->shoot(i, shootPoint); //使用火球

		Point meteorPoint = getExtrapolatedLocation(targetHeroIndex, 1.9 + 0.05 * i);
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
	int startTimeStamp = getCurrentTimeStamp();
	for (int i = 0; i < 5; i++)
	{
		Point myPostion = GetUnit(i).position;

		Point finalDest = dest[i];

		if (isPointValid(highLevelPoints[i]))
		{
			finalDest = highLevelPoints[i];
		}

		int currentPointRisk = getRiskMapValue(myPostion);

		if (myPostion == finalDest and currentPointRisk != 0)
		{
			finalDest = findNearbyNoRiskLocation(finalDest);
		}
		if (isWall(finalDest))
		{
			finalDest = findNearbyMovableLocation(finalDest);
		}
		if (myPostion != finalDest or dist(myPostion, finalDest) > 0.5)
		{
			if (isLineWalkable(myPostion, finalDest) and currentPointRisk == 0)
			{
				// Point oldTargetVector = (finalDest - myPostion) / dist(myPostion, finalDest);
				// Point fixVector = getAvoidVector(myPostion);
				// Point target = myPostion + oldTargetVector + fixVector;
				// move_s(i, target);
				move_s(i, finalDest);
				if (dist(myPostion, finalDest) >= CONST::flash_distance / 2 and canflash(i) and !isWall(finalDest))
				{
					flash_s(i, finalDest);
					pathPointsNew[i] = Point(-1, -1);
				}
				mylog._info("	Direct Line for (%lf,%lf) to (%lf,%lf),walk through", myPostion.x, myPostion.y, finalDest.x, finalDest.y);
			}
			else
			{
				move_greed(i, finalDest);
			}
		}
		else
		{
			Point fixVector = getAvoidVector(myPostion);
			Point target = myPostion + fixVector;
			move_s(i, target);
			mylog._info("Player %d arrival target (%lf,%lf)", i, finalDest.x, finalDest.y);
		}
	}
	int endTimeStamp = getCurrentTimeStamp();
	mylog.write(YFL, LOG_INFO, "[executeMove] costTime:%dms", endTimeStamp - startTimeStamp);
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
	generateRiskMap();
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