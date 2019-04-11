#pragma once

namespace CONST {
	const int frames_per_second = 20;
	const double time_of_round = 1.0 / frames_per_second;
	const int frames_of_death = 8 * frames_per_second;
	const int frames_of_invincible = 2 * frames_per_second;

	const int human_hp = 100;
	const int human_meteor_number = 1000;
	const int human_flash_number = 1000;
	const int human_fireball_interval = frames_per_second * 0.5;
	const int human_meteor_interval = frames_per_second * 8;
	const int human_flash_interval = frames_per_second * 3;
	const double human_velocity = 0.6;

	const double fireball_velocity = 3;
	const double fireball_radius = 3;
	const double splash_radius = 3;
	const int splash_hurt = 5;

	const double meteor_distance = 30;
	const int meteor_delay = frames_per_second * 2;
	const double explode_radius = 3;
	const int explode_hurt = 100;

	const double flash_distance = 20;

	const double ball_radius = 3;

	const double target_radius = 5;

	const int bonus_radius = 6;
	const int bonus_time_low = 8 * frames_per_second;
	const int bonus_time_high = 12 * frames_per_second;

	const int kill_score = 1;
	const int killed_score = 0;
	const int goal_score = 80;
	const int goaled_score = 0;
	const int bonus_score = 10;


	const double room_size = 1;

}