#include "logic.h"
#include <iostream>
using namespace std;

Logic* Logic::instance = 0;

void Logic::move(int num, Point p){
	ope.move[num] = p;
}

void Logic::shoot(int num, Point p){
	ope.shoot[num] = p;
}

void Logic::meteor(int num, Point p){
	ope.meteor[num] = p;
}

void Logic::flash(int num){
	ope.flash[num] = true;
}

void Logic::unmove(int num){
	ope.move[num] = Point(-1, -1);
}

void Logic::unshoot(int num){
	ope.shoot[num] = Point(-1, -1);
}

void Logic::unmeteor(int num){
	ope.meteor[num] = Point(-1, -1);
}

void Logic::unflash(int num){
	ope.flash[num] = false;
}

bool Logic::isWall(int x, int y){
	return map.pixels[x][y];
}

void Logic::initMap(int w, int h, int f, int hn, vector<vector<Point>> b, vector<Point> c, vector<Point> t, vector<Point> bo, vector<vector<bool>> p, int ti) {
	return map.set(w, h, f, hn, b, c, t, bo, p, ti);
}

void Logic::getFrame(int f, vector<Human> h, vector<Fireball> b, vector<Meteor> g, vector<Crystal> ba, vector<bool> bo) {
	frame = f;
	humans = h;
	fireballs = b;
	meteors = g;
	crystal = ba;
	bonus = bo;
}

void Logic::resetOpe() {
	ope.flag = 0;
	ope.move = vector<Point>(map.human_number, Point(-1, -1));
	ope.shoot = vector<Point>(map.human_number, Point(-1, -1));
	ope.meteor = vector<Point>(map.human_number, Point(-1, -1));
	ope.flash = vector<bool>(map.human_number, false);
}