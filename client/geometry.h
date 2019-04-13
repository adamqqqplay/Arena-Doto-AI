#pragma once

/* 需要包含的头文件 */
#include <cmath> 
#include <algorithm>

/* 常用的常量定义 */
const double	INF = 1E200;
const double	EP = 1E-10;
const int		MAXV = 300;
const double	PI = 3.14159265;

using namespace std;
/* 基本几何结构 */

//点
struct Point {
	double x, y;
	Point() {};
	Point(double a, double b) { x = a; y = b; }
};

//两点式
struct Lineseg {
	Point s;
	Point e;
	Lineseg(Point a, Point b) { s = a; e = b; }
	Lineseg() { }
};

//一般式
struct Line {         // 直线的解析方程 a*x+b*y+c=0  为统一表示，约定 a >= 0 
	double a;
	double b;
	double c;
	Line(double d1 = 1, double d2 = -1, double d3 = 0) { a = d1; b = d2; c = d3; }
};




/**********************
*                    *
*   点的基本运算     *
*                    *
**********************/

// 返回两点之间欧氏距离 
double dist(Point p1, Point p2); 

// 判断两个点是否重合 
bool equal_Point(Point p1, Point p2);

/******************************************************************************
r=multiply(sp,ep,op),得到(sp-op)和(ep-op)的叉积
r>0：ep在矢量opsp的逆时针方向；
r=0：opspep三点共线；
r<0：ep在矢量opsp的顺时针方向
*******************************************************************************/
double multiply(Point sp, Point ep, Point op);

/******************************************************************************
r=dotmultiply(p1,p2,op),得到矢量(p1-op)和(p2-op)的点积，如果两个矢量都非零矢量
r<0：两矢量夹角为锐角；
r=0：两矢量夹角为直角；
r>0：两矢量夹角为钝角
*******************************************************************************/
double dotmultiply(Point p1, Point p2, Point p0);

/******************************************************************************
判断点p是否在线段l上
条件：(p在线段l所在的直线上) && (点p在以线段l为对角线的矩形内)
*******************************************************************************/
bool onLine(Lineseg l, Point p);

/******************************************************************************
返回点p以点o为圆心逆时针旋转alpha(单位：弧度)后所在的位置
*******************************************************************************/
Point rotate(Point o, double alpha, Point p);

/******************************************************************************
返回以点o为圆心的圆上坐标
*******************************************************************************/
Point getNearbyPoint(Point o, double alpha, double radius);

/******************************************************************************
返回以圆心到圆的向量
*******************************************************************************/
Point getNearbyVector(double alpha, double radius);

//弧度转角度
double rad2deg(double x);

//角度转弧度
double deg2rad(double x);

/******************************************************************************
返回顶角在o点，起始边为os，终止边为oe的夹角(单位：弧度)
角度小于pi，返回正值
角度大于pi，返回负值
可以用于求线段之间的夹角
原理：
r = dotmultiply(s,e,o) / (dist(o,s)*dist(o,e))
r'= multiply(s,e,o)
r >= 1	angle = 0;
r <= -1	angle = -PI
-1<r<1 && r'>0	angle = arccos(r)
-1<r<1 && r'<=0	angle = -arccos(r)
*******************************************************************************/
double angle(Point o, Point s, Point e);


/*****************************\
*                             *
*     线段及直线的基本运算    *
*                             *
\*****************************/


/******************************************************************************
判断点与线段的关系
本函数是根据下面的公式写的，P是点C到线段AB所在直线的垂足
AC dot AB
r =     ---------
||AB||^2
(Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)
= -------------------------------
L^2
r has the following meaning:
r=0      P = A
r=1      P = B
r<0		 P is on the backward extension of AB
r>1      P is on the forward extension of AB
0<r<1	 P is interior to AB
*******************************************************************************/
double relation(Point p, Lineseg l);

/******************************************************************************
求点C到线段AB所在直线的垂足 P
*******************************************************************************/
Point perpendicular(Point p, Lineseg l);

/******************************************************************************
求点p到线段l的最短距离,并返回线段上距该点最近的点np
注意：np是线段l上到点p最近的点，不一定是垂足
*******************************************************************************/
double ptoLinesegdist(Point p, Lineseg l, Point &np);

// 求点p到线段l所在直线的距离,请注意本函数与上个函数的区别  
double ptoldist(Point p, Lineseg l);

/* 计算点到折线集的最近距离,并返回最近点.
注意：调用的是ptoLineseg()函数 */
double ptoPointset(int vcount, Point Pointset[], Point p, Point &q);

/* 判断圆是否在多边形内.ptoLineseg()函数的应用2 */
bool CircleInsidePolygon(int vcount, Point center, double radius, Point polygon[]);

/* 返回两个矢量l1和l2的夹角的余弦(-1 --- 1)注意：如果想从余弦求夹角的话，注意反余弦函数的定义域是从 0到pi */
double cosine(Lineseg l1, Lineseg l2);
// 返回线段l1与l2之间的夹角 单位：弧度 范围(-pi，pi) 
double lsangle(Lineseg l1, Lineseg l2);

//如果线段u和v相交(包括相交在端点处)时，返回true 
//判断P1P2跨立Q1Q2的依据是：( P1 - Q1 ) × ( Q2 - Q1 ) * ( Q2 - Q1 ) × ( P2 - Q1 ) >= 0。
//判断Q1Q2跨立P1P2的依据是：( Q1 - P1 ) × ( P2 - P1 ) * ( P2 - P1 ) × ( Q2 - P1 ) >= 0。
bool intersect(Lineseg u, Lineseg v);

//  (线段u和v相交)&&(交点不是双方的端点) 时返回true    
bool intersect_A(Lineseg u, Lineseg v);

// 线段v所在直线与线段u相交时返回true；方法：判断线段u是否跨立线段v  
bool intersect_l(Lineseg u, Lineseg v);

// 根据已知两点坐标，求过这两点的直线解析方程： a*x+b*y+c = 0  (a >= 0)  
Line makeLine(Point p1, Point p2);

// 根据直线解析方程返回直线的斜率k,水平线返回 0,竖直线返回 1e200 
double slope(Line l);

// 返回直线的倾斜角alpha ( 0 - pi) 
double alpha(Line l);

// 求点p关于直线l的对称点  
Point symmetry(Line l, Point p);

// 如果两条直线 l1(a1*x+b1*y+c1 = 0), l2(a2*x+b2*y+c2 = 0)相交，返回true，且返回交点p  
bool Lineintersect(Line l1, Line l2, Point &p);

// 如果线段l1和l2相交，返回true且交点由(inter)返回，否则返回false 
bool intersection(Lineseg l1, Lineseg l2, Point &inter);

/*************************\
*						 *
* 圆的基本运算           *
*					     *
\*************************/
/******************************************************************************
返回值	： 点p在圆内(包括边界)时，返回true
用途	： 因为圆为凸集，所以判断点集，折线，多边形是否在圆内时，
只需要逐一判断点是否在圆内即可。
*******************************************************************************/
bool Point_in_circle(Point o, double r, Point p);

//两圆关系： 
/* 两圆：
相离： return 1；
外切： return 2；
相交： return 3；
内切： return 4；
内含： return 5；
*/
int CircleRelation(Point p1, double r1, Point p2, double r2);

/**************************\
*						  *
* 矩形的基本运算          *
*                         *
\**************************/

// 已知矩形的三个顶点(a,b,c)，计算第四个顶点d的坐标. 注意：已知的三个顶点可以是无序的 
Point rect4th(Point a, Point b, Point c);

/********************\
*				    *
* 补充				*
*					*
\********************/

//判断圆是否在矩形内：
// 判定圆是否在矩形内，是就返回true（设矩形水平，且其四个顶点由左上开始按顺时针排列） =
bool CircleRecRelation(Point pc, double r, Point pr1, Point pr2, Point pr3, Point pr4);

//点是否在直线同侧：
//两个点是否在直线同侧，是则返回true 
bool SameSide(Point p1, Point p2, Line Line);

