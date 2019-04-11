import os
import random
import json
import numpy as np
# some constants about pygame
black = (0, 0, 0)
white = (255, 255, 255)
red = (255, 0, 0)
green = (0, 255, 0)
blue = (0, 0, 255)
yellow = (255, 255, 0)
golden = (217, 217, 25)
gray = (127, 127, 127)
purple = (255, 0, 255)
pink = (242, 156, 177)


# game setting
friendly_fire = False

# precision
precision = 3

# load the map,assert the map's legality
map_dir = os.getcwd() + os.sep + "Maps" + os.sep
map_names = os.listdir(map_dir)
map_names.sort()
map_id = random.randint(0, len(map_names) - 1)
map_name = map_names[map_id]
with open(map_dir + map_name, "r")as file:
    JSON = file.read()
    JSON = json.loads(JSON)
width = JSON["width"]
height = JSON["height"]
faction_number = JSON["faction_number"]
birth_places = JSON["birth_places"]
ball_places = JSON["ball_places"]
bonus_places = JSON["bonus_places"]
target_places = JSON["target_places"]
walls = JSON["walls"]
walls = np.asarray(walls).astype(np.bool)
human_number = JSON["human_number"]
room_size = 1


# some constants about time
frames_per_second = 20
frames_of_game = JSON["time_of_game"] * frames_per_second
time_of_round = 1.0 / frames_per_second
frames_of_death = 8 * frames_per_second
frames_of_invincible = 2 * frames_per_second

# some constants about human
human_hp = 100
human_meteor_number = 1000
human_flash_number = 1000
human_fireball_interval = frames_per_second * 0.5
human_meteor_interval = frames_per_second * 8
human_flash_interval = frames_per_second * 3
human_velocity = 0.6

# some constants about skills
fireball_velocity = 3
fireball_radius = 3
splash_radius = 3
splash_hurt = 5

meteor_distance = 30
meteor_delay = frames_per_second * 2
explode_radius = 3
explode_hurt = 100

flash_distance = 20

# some constants about balls
ball_radius = 3

# some constants about target district
target_radius = 5

# some constants about bonus district
bonus_radius = 6
bonus_time_low = 8 * frames_per_second
bonus_time_high = 12 * frames_per_second

# some constants about score
kill_score = 1
killed_score = 0
goal_score = 80
goaled_score = 0
bonus_score = 10
