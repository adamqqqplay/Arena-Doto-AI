#pragma once

namespace CONST {
	//帧率 20fps
	const int frames_per_second = 20;
	//每帧时间 0.05s
	const double time_of_round = 1.0 / frames_per_second;
	//死亡时间 8s 帧数160
	const int frames_of_death = 8 * frames_per_second;
	//无敌时间 2s 帧数40
	const int frames_of_invincible = 2 * frames_per_second;

	//单位血量 100
	const int human_hp = 100;
	//英雄移动速度12 每帧移动0.6
	const double human_velocity = 0.6;
	
	//陨石次数限制 1000
	const int human_meteor_number = 1000;
	//闪现次数限制 1000
	const int human_flash_number = 1000;


	//火球弹道速度60 每帧火球移动3
	const double fireball_velocity = 3;
	//火球触发范围3
	const double fireball_radius = 3;
	//火球作用范围3
	const double splash_radius = 3;
	//火球爆炸伤害5
	const int splash_hurt = 5;
	//火球冷却时间 0.5s
	const int human_fireball_interval = frames_per_second * 0.5;

	//陨石施法距离30
	const double meteor_distance = 30;
	//陨石生效延迟2s
	const int meteor_delay = frames_per_second * 2;
	//陨石作用范围3
	const double explode_radius = 3;
	//陨石爆炸伤害100
	const int explode_hurt = 100;
	//陨石冷却时间 8s
	const int human_meteor_interval = frames_per_second * 8;

	//闪现距离20
	const double flash_distance = 20;
	//闪现冷却时间 3s
	const int human_flash_interval = frames_per_second * 3;

	//水晶拾取范围3
	const double ball_radius = 3;
	//目标水晶范围5
	const double target_radius = 5;

	//赏金符拾取范围6
	const int bonus_radius = 6;
	//最短生成时间8s
	const int bonus_time_low = 8 * frames_per_second;
	//最长生成时间12s
	const int bonus_time_high = 12 * frames_per_second;

	//击杀得分1
	const int kill_score = 1;
	//死亡得分0
	const int killed_score = 0;
	//水晶得分80
	const int goal_score = 80;
	//失去水晶得分0
	const int goaled_score = 0;
	//赏金符得分10
	const int bonus_score = 10;

	//地图网格
	const double room_size = 1;
}