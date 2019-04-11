#include "playerAI.h"
#include<queue>
#include<cstring>
#include<algorithm>
#include<ctime>
using namespace std;

Logic * logic;

int node_translate(int faction,int num)
{
	return num*logic->map.faction_number+faction;
}
Human GetUnit(int num)
{
	return logic->humans[node_translate(logic->faction,num)];
}
Human GetEnemyUnit(int num)
{
	return logic->humans[node_translate(logic->faction^1,num)];
}
Human GetUnit(int faction,int num)
{
	return logic->humans[node_translate(faction,num)];
}
bool isWall(int x,int y)
{
	if(x<0||y<0||x>=logic->map.width||y>=logic->map.height)return true;
	return logic->map.pixels[x][y];
}
bool isWall(Point v)
{
	return isWall(v.x,v.y);
}

void move(int num,Point v)
{
	if(!isWall(v))logic->move(num,v);
}
void move_relative(int num,Point v)
{
	double mod2=v.x*v.x+v.y*v.y;
	if(mod2>(CONST::human_velocity-0.0001)*(CONST::human_velocity-0.0001))
	{
		mod2=sqrt(mod2);
		mod2=(CONST::human_velocity-0.0001)/mod2;
		v.x*=mod2;
		v.y*=mod2;
	}
	Point pos=GetUnit(num).position;
	v.x+=pos.x;
	v.y+=pos.y;
	move(num,v);
}
void move_s(int num, Point v)
{
	Point pos=GetUnit(num).position;
	v.x-=pos.x;
	v.y-=pos.y;
	move_relative(num,v);
}
int dis[320][320],inf_dis;
queue<pair<int,int>> Q;
#define mp(a,b) make_pair((a),(b))
void move_stupid(int num,Point v)
{
	memset(dis,127,sizeof dis);
	memset(&inf_dis,127,sizeof(int));
	Point pos=GetUnit(num).position;
	Q.push(mp((int)v.x,(int)v.y));
	dis[(int)v.x][(int)v.y]=0;
	while(!Q.empty())
	{
		pair<int,int> st=Q.front();Q.pop();
		int D=dis[st.first][st.second];
		for(int direct=0;direct<4;direct++)
		{
			int dx=0,dy=0,x,y;
			if(direct==0)dx=-1;
			if(direct==1)dx=1;
			if(direct==2)dy=-1;
			if(direct==3)dy=1;
			x=dx+st.first;
			y=dy+st.second;
			if(!isWall(x,y)&&dis[x][y]==inf_dis)
			{
				dis[x][y]=D+1;
				Q.push(mp(x,y));
			}
		}
	}
	//move_s(num,v);
	//return;
	int px=pos.x,py=pos.y;
	int base=rand()%4;
	for(int i=0;i<4;i++)
	{
		int direct=(i+base)%4;
		int dx=0,dy=0,x,y;
		if(direct==0)dx=-1;
		if(direct==1)dx=1;
		if(direct==2)dy=-1;
		if(direct==3)dy=1;
		x=px+dx;y=py+dy;
		if(dis[x][y]<dis[px][py])
		{
			if(dis[px+dx*2][py+dy*2]<dis[x][y])dx*=2,dy*=2;
			move_relative(num,Point(dx,dy));
			return;
		}
	}
}
bool canflash(int num)
{
	return GetUnit(num).flash_time<=0&&logic->crystal[logic->faction^1].belong!=node_translate(logic->faction,num);
}
void flash(int num,Point v)
{
	if(!canflash(num))return;
	if(isWall(v))return;
	logic->flash(num);
	logic->move(num,v);
}
void flash_relative(int num,Point v)
{
	double mod2=v.x*v.x+v.y*v.y;
	if(mod2>(CONST::flash_distance-0.0001)*(CONST::flash_distance-0.0001))
	{
		mod2=sqrt(mod2);
		mod2=(CONST::flash_distance-0.0001)/mod2;
		v.x*=mod2;
		v.y*=mod2;
	}
	Point pos=GetUnit(num).position;
	v.x+=pos.x;
	v.y+=pos.y;
	flash(num,v);
}
void flash_s(int num, Point v)
{
	Point pos=GetUnit(num).position;
	v.x-=pos.x;
	v.y-=pos.y;
	flash_relative(num,v);
}
double RandDouble(){return rand()/(double)RAND_MAX;}
double RandDouble(double L,double R){return RandDouble()*(R-L)+L;}
/*double dist(Point p1,Point p2)
{
	p1.x-=p2.x;
	p1.y-=p2.y;
	return sqrt(p1.x*p1.x+p1.y*p1.y);
}//*/

void playerAI() {
	logic = Logic::Instance();
	if(logic->frame==1)srand(time(NULL));
	Point dest[5]={logic->crystal[logic->faction].position,logic->map.bonus_places[0],logic->map.bonus_places[1]},rush=logic->crystal[logic->faction^1].position;
	if(~logic->crystal[logic->faction^1].belong)rush=logic->map.target_places[logic->faction];
	dest[4]=dest[3]=rush;
	for(int i=0;i<5;i++)
		move_stupid(i,dest[i]),flash_s(i,dest[i]);
	for(int i=0;i<5;i++)
	{
		Point mypos=GetUnit(i).position;
		Point targ=logic->map.birth_places[logic->faction^1][0];
		for(int j=0;j<5;j++)
		{
			Human unit=GetEnemyUnit(j);
			if(unit.hp<=0)continue;
			if(dist(unit.position,mypos)<dist(targ,mypos))targ=unit.position;
		}
		double D=dist(targ,mypos)*0.2;
		logic->shoot(i,Point(targ.x+RandDouble(-D,D),targ.y+RandDouble(-D,D)));
		logic->meteor(i,Point(targ.x+RandDouble(-10,10),targ.y+RandDouble(-10,10)));
	}
}