#include "logic.h"
#include <iostream>
using namespace std;

Logic* Logic::instance = 0;
//指定你控制的第num个人移动到p位置
void Logic::move(int num, Point p){
	ope.move[num] = p;
}
//指定你控制的第num个人向p位置发射火球
void Logic::shoot(int num, Point p){
	ope.shoot[num] = p;
}
//指定你控制的第num个人向p位置释放陨石术
void Logic::meteor(int num, Point p){
	ope.meteor[num] = p;
}
//指定你控制的第num个人本次移动改为闪现
void Logic::flash(int num){
	ope.flash[num] = true;
}
//取消你控制的第num个人的移动指令
void Logic::unmove(int num){
	ope.move[num] = Point(-1, -1);
}
//取消你控制的第num个人的射击指令
void Logic::unshoot(int num){
	ope.shoot[num] = Point(-1, -1);
}
//取消你控制的第num个人的发射陨石指令
void Logic::unmeteor(int num){
	ope.meteor[num] = Point(-1, -1);
}
//取消你控制的第num个人的闪现指令
void Logic::unflash(int num){
	ope.flash[num] = false;
}
//判断该点是否为墙
bool Logic::isWall(int x, int y){
	return map.pixels[x][y];
}
//初始化地图
void Logic::initMap(int w, int h, int f, int hn, vector<vector<Point>> b, vector<Point> c, vector<Point> t, vector<Point> bo, vector<vector<bool>> p, int ti) {
	debugMsg = "";
	return map.set(w, h, f, hn, b, c, t, bo, p, ti);
}
//获取帧
void Logic::getFrame(int f, vector<Human> h, vector<Fireball> b, vector<Meteor> g, vector<Crystal> ba, vector<bool> bo) {
	frame = f;
	humans = h;
	fireballs = b;
	meteors = g;
	crystal = ba;
	bonus = bo;
}
//重置所有操作
void Logic::resetOpe() {
	ope.flag = 0;
	ope.move = vector<Point>(map.human_number, Point(-1, -1));
	ope.shoot = vector<Point>(map.human_number, Point(-1, -1));
	ope.meteor = vector<Point>(map.human_number, Point(-1, -1));
	ope.flash = vector<bool>(map.human_number, false);
}

void Logic::debug(string msg) {
	debugMsg = msg;
}

void Logic::debugAppend(string amsg) {
	debugMsg += amsg;
}