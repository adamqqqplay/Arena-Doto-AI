#include "geometry.h"


/**********************
*                    *
*   点的基本运算     *
*                    *
**********************/

double dist(Point p1, Point p2) {                // 返回两点之间欧氏距离 
	return(sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y)));
}

bool equal_Point(Point p1, Point p2) {           // 判断两个点是否重合  
	return ((abs(p1.x - p2.x)<EP) && (abs(p1.y - p2.y)<EP));
}

/******************************************************************************
r=multiply(sp,ep,op),得到(sp-op)和(ep-op)的叉积
r>0：ep在矢量opsp的逆时针方向；
r=0：opspep三点共线；
r<0：ep在矢量opsp的顺时针方向
*******************************************************************************/
double multiply(Point sp, Point ep, Point op) {
	return((sp.x - op.x)*(ep.y - op.y) - (ep.x - op.x)*(sp.y - op.y));
}

/******************************************************************************
r=dotmultiply(p1,p2,op),得到矢量(p1-op)和(p2-op)的点积，如果两个矢量都非零矢量
r<0：两矢量夹角为锐角；
r=0：两矢量夹角为直角；
r>0：两矢量夹角为钝角
*******************************************************************************/
double dotmultiply(Point p1, Point p2, Point p0) {
	return ((p1.x - p0.x)*(p2.x - p0.x) + (p1.y - p0.y)*(p2.y - p0.y));
}

/******************************************************************************
判断点p是否在线段l上
条件：(p在线段l所在的直线上) && (点p在以线段l为对角线的矩形内)
*******************************************************************************/
bool onLine(Lineseg l, Point p) {
	return((multiply(l.e, p, l.s) == 0) && (((p.x - l.s.x)*(p.x - l.e.x) <= 0) && ((p.y - l.s.y)*(p.y - l.e.y) <= 0)));
}

/******************************************************************************
返回点p以点o为圆心逆时针旋转alpha(单位：弧度)后所在的位置
*******************************************************************************/
Point rotate(Point o, double alpha, Point p) {
	Point tp;
	p.x -= o.x;
	p.y -= o.y;
	tp.x = p.x*cos(alpha) - p.y*sin(alpha) + o.x;
	tp.y = p.y*cos(alpha) + p.x*sin(alpha) + o.y;
	return tp;
}

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
double angle(Point o, Point s, Point e) {
	double cosfi, fi, norm;
	double dsx = s.x - o.x;
	double dsy = s.y - o.y;
	double dex = e.x - o.x;
	double dey = e.y - o.y;

	cosfi = dsx * dex + dsy * dey;
	norm = (dsx*dsx + dsy * dsy)*(dex*dex + dey * dey);
	cosfi /= sqrt(norm);

	if (cosfi >= 1.0) return 0;
	if (cosfi <= -1.0) return -3.1415926;

	fi = acos(cosfi);
	if (dsx*dey - dsy * dex>0) return fi;      // 说明矢量os 在矢量 oe的顺时针方向 
	return -fi;
}


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
double relation(Point p, Lineseg l) {
	Lineseg tl;
	tl.s = l.s;
	tl.e = p;
	return dotmultiply(tl.e, l.e, l.s) / (dist(l.s, l.e)*dist(l.s, l.e));
}

/******************************************************************************
求点C到线段AB所在直线的垂足 P
*******************************************************************************/
Point perpendicular(Point p, Lineseg l) {
	double r = relation(p, l);
	Point tp;
	tp.x = l.s.x + r * (l.e.x - l.s.x);
	tp.y = l.s.y + r * (l.e.y - l.s.y);
	return tp;
}

/******************************************************************************
求点p到线段l的最短距离,并返回线段上距该点最近的点np
注意：np是线段l上到点p最近的点，不一定是垂足
*******************************************************************************/
double ptoLinesegdist(Point p, Lineseg l, Point &np) {
	double r = relation(p, l);
	if (r<0) {
		np = l.s;
		return dist(p, l.s);
	}
	if (r>1) {
		np = l.e;
		return dist(p, l.e);
	}
	np = perpendicular(p, l);
	return dist(p, np);
}

// 求点p到线段l所在直线的距离,请注意本函数与上个函数的区别  
double ptoldist(Point p, Lineseg l) {
	return abs(multiply(p, l.e, l.s)) / dist(l.s, l.e);
}

/* 计算点到折线集的最近距离,并返回最近点.
注意：调用的是ptoLineseg()函数 */
double ptoPointset(int vcount, Point Pointset[], Point p, Point &q) {
	int i;
	double cd = double(INF), td;
	Lineseg l;
	Point tq, cq;

	for (i = 0; i<vcount - 1; i++) {
		l.s = Pointset[i];

		l.e = Pointset[i + 1];
		td = ptoLinesegdist(p, l, tq);
		if (td<cd) {
			cd = td;
			cq = tq;
		}
	}
	q = cq;
	return cd;
}

/* 判断圆是否在多边形内.ptoLineseg()函数的应用2 */
bool CircleInsidePolygon(int vcount, Point center, double radius, Point polygon[]) {
	Point q;
	double d;
	q.x = 0;
	q.y = 0;
	d = ptoPointset(vcount, polygon, center, q);
	if (d<radius || fabs(d - radius)<EP)
		return true;
	else
		return false;
}

/* 返回两个矢量l1和l2的夹角的余弦(-1 --- 1)注意：如果想从余弦求夹角的话，注意反余弦函数的定义域是从 0到pi */
double cosine(Lineseg l1, Lineseg l2) {
	return (((l1.e.x - l1.s.x)*(l2.e.x - l2.s.x) +
		(l1.e.y - l1.s.y)*(l2.e.y - l2.s.y)) / (dist(l1.e, l1.s)*dist(l2.e, l2.s)));
}

// 返回线段l1与l2之间的夹角 单位：弧度 范围(-pi，pi) 
double lsangle(Lineseg l1, Lineseg l2) {
	Point o, s, e;
	o.x = o.y = 0;
	s.x = l1.e.x - l1.s.x;
	s.y = l1.e.y - l1.s.y;
	e.x = l2.e.x - l2.s.x;
	e.y = l2.e.y - l2.s.y;
	return angle(o, s, e);
}

//如果线段u和v相交(包括相交在端点处)时，返回true 
//判断P1P2跨立Q1Q2的依据是：( P1 - Q1 ) × ( Q2 - Q1 ) * ( Q2 - Q1 ) × ( P2 - Q1 ) >= 0。
//判断Q1Q2跨立P1P2的依据是：( Q1 - P1 ) × ( P2 - P1 ) * ( P2 - P1 ) × ( Q2 - P1 ) >= 0。
bool intersect(Lineseg u, Lineseg v) {
	return((max(u.s.x, u.e.x) >= min(v.s.x, v.e.x)) &&                     //排斥实验 
		(max(v.s.x, v.e.x) >= min(u.s.x, u.e.x)) &&
		(max(u.s.y, u.e.y) >= min(v.s.y, v.e.y)) &&
		(max(v.s.y, v.e.y) >= min(u.s.y, u.e.y)) &&
		(multiply(v.s, u.e, u.s)*multiply(u.e, v.e, u.s) >= 0) &&         //跨立实验 
		(multiply(u.s, v.e, v.s)*multiply(v.e, u.e, v.s) >= 0));
}

//  (线段u和v相交)&&(交点不是双方的端点) 时返回true    
bool intersect_A(Lineseg u, Lineseg v) {
	return	((intersect(u, v)) &&
		(!onLine(u, v.s)) &&
		(!onLine(u, v.e)) &&
		(!onLine(v, u.e)) &&
		(!onLine(v, u.s)));
}

// 线段v所在直线与线段u相交时返回true；方法：判断线段u是否跨立线段v  
bool intersect_l(Lineseg u, Lineseg v) {
	return multiply(u.s, v.e, v.s)*multiply(v.e, u.e, v.s) >= 0;
}

// 根据已知两点坐标，求过这两点的直线解析方程： a*x+b*y+c = 0  (a >= 0)  
Line makeLine(Point p1, Point p2) {
	Line tl;
	int sign = 1;
	tl.a = p2.y - p1.y;
	if (tl.a<0) {
		sign = -1;
		tl.a = sign * tl.a;
	}
	tl.b = sign * (p1.x - p2.x);
	tl.c = sign * (p1.y*p2.x - p1.x*p2.y);
	return tl;
}
// 根据直线解析方程返回直线的斜率k,水平线返回 0,竖直线返回 1e200 
double slope(Line l) {
	if (abs(l.a) < 1e-20)
		return 0;
	if (abs(l.b) < 1e-20)
		return INF;
	return -(l.a / l.b);
}
// 返回直线的倾斜角alpha ( 0 - pi) 
double alpha(Line l) {
	if (abs(l.a)< EP)
		return 0;
	if (abs(l.b)< EP)
		return PI / 2;
	double k = slope(l);
	if (k>0)
		return atan(k);
	else
		return PI + atan(k);
}
// 求点p关于直线l的对称点  
Point symmetry(Line l, Point p) {
	Point tp;
	tp.x = ((l.b*l.b - l.a*l.a)*p.x - 2 * l.a*l.b*p.y - 2 * l.a*l.c) / (l.a*l.a + l.b*l.b);
	tp.y = ((l.a*l.a - l.b*l.b)*p.y - 2 * l.a*l.b*p.x - 2 * l.b*l.c) / (l.a*l.a + l.b*l.b);
	return tp;
}
// 如果两条直线 l1(a1*x+b1*y+c1 = 0), l2(a2*x+b2*y+c2 = 0)相交，返回true，且返回交点p  
bool Lineintersect(Line l1, Line l2, Point &p) {// 是 L1，L2  
	double d = l1.a*l2.b - l2.a*l1.b;
	if (abs(d)<EP) // 不相交 
		return false;
	p.x = (l2.c*l1.b - l1.c*l2.b) / d;
	p.y = (l2.a*l1.c - l1.a*l2.c) / d;
	return true;
}
// 如果线段l1和l2相交，返回true且交点由(inter)返回，否则返回false 
bool intersection(Lineseg l1, Lineseg l2, Point &inter) {
	Line ll1, ll2;
	ll1 = makeLine(l1.s, l1.e);
	ll2 = makeLine(l2.s, l2.e);
	if (Lineintersect(ll1, ll2, inter))
		return onLine(l1, inter);
	else
		return false;
}

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
bool Point_in_circle(Point o, double r, Point p) {
	double d2 = (p.x - o.x)*(p.x - o.x) + (p.y - o.y)*(p.y - o.y);
	double r2 = r * r;
	return d2<r2 || abs(d2 - r2)<EP;
}


/**************************\
*						  *
* 矩形的基本运算          *
*                         *
\**************************/

// 已知矩形的三个顶点(a,b,c)，计算第四个顶点d的坐标. 注意：已知的三个顶点可以是无序的 
Point rect4th(Point a, Point b, Point c) {
	Point d;
	if (abs(dotmultiply(a, b, c))<EP) { // 说明c点是直角拐角处
		d.x = a.x + b.x - c.x;
		d.y = a.y + b.y - c.y;
	}
	if (abs(dotmultiply(a, c, b))<EP) { // 说明b点是直角拐角处 
		d.x = a.x + c.x - b.x;
		d.y = a.y + c.y - b.x;
	}
	if (abs(dotmultiply(c, b, a))<EP) { // 说明a点是直角拐角处 
		d.x = c.x + b.x - a.x;
		d.y = c.y + b.y - a.y;
	}
	return d;
}

/********************\
*				    *
* 补充				*
*					*
\********************/

//两圆关系： 
/* 两圆：
相离： return 1；
外切： return 2；
相交： return 3；
内切： return 4；
内含： return 5；
*/
int CircleRelation(Point p1, double r1, Point p2, double r2) {
	double d = sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));

	if (fabs(d - r1 - r2) < EP) // 必须保证前两个if先被判定！ 
		return 2;
	if (fabs(d - fabs(r1 - r2)) < EP)
		return 4;
	if (d > r1 + r2)
		return 1;
	if (d < fabs(r1 - r2))
		return 5;
	if (fabs(r1 - r2) < d && d < r1 + r2)
		return 3;
	return 0; // indicate an error! 
}
//判断圆是否在矩形内：
// 判定圆是否在矩形内，是就返回true（设矩形水平，且其四个顶点由左上开始按顺时针排列） 
// 调用ptoldist函数，在第4页 
bool CircleRecRelation(Point pc, double r, Point pr1, Point pr2, Point pr3, Point pr4) {
	if (pr1.x < pc.x && pc.x < pr2.x && pr3.y < pc.y && pc.y < pr2.y) {
		Lineseg Line1(pr1, pr2);
		Lineseg Line2(pr2, pr3);
		Lineseg Line3(pr3, pr4);
		Lineseg Line4(pr4, pr1);
		if (r<ptoldist(pc, Line1) && r<ptoldist(pc, Line2) && r<ptoldist(pc, Line3) && r<ptoldist(pc, Line4))
			return true;
	}
	return false;
}

//点是否在直线同侧：
//两个点是否在直线同侧，是则返回true 
bool SameSide(Point p1, Point p2, Line Line) {
	return (Line.a * p1.x + Line.b * p1.y + Line.c) *
		(Line.a * p2.x + Line.b * p2.y + Line.c) > 0;
}

//镜面反射线：
// 已知入射线、镜面，求反射线。 
// a1,b1,c1为镜面直线方程(a1 x + b1 y + c1 = 0 ,下同)系数;  
//a2,b2,c2为入射光直线方程系数;  
//a,b,c为反射光直线方程系数. 
// 光是有方向的，使用时注意：入射光向量:<-b2,a2>；反射光向量:<b,-a>. 
// 不要忘记结果中可能会有"negative zeros" 
void reflect(double a1, double b1, double c1, double a2, double b2, double c2, double &a, double &b, double &c) {
	double n, m;
	double tpb, tpa;
	tpb = b1 * b2 + a1 * a2;
	tpa = a2 * b1 - a1 * b2;
	m = (tpb*b1 + tpa * a1) / (b1*b1 + a1 * a1);
	n = (tpa*b1 - tpb * a1) / (b1*b1 + a1 * a1);
	if (fabs(a1*b2 - a2 * b1)<1e-20) {
		a = a2; b = b2; c = c2;
		return;
	}
	double xx, yy; //(xx,yy)是入射线与镜面的交点。 
	xx = (b1*c2 - b2 * c1) / (a1*b2 - a2 * b1);
	yy = (a2*c1 - a1 * c2) / (a1*b2 - a2 * b1);
	a = n;
	b = -m;
	c = m * yy - xx * n;
}
