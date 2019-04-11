import sys
import os
import random
import copy
import time
import json
import numpy as np

scale = 1.0

if __name__ == "__main__":
    replay_dir = "." + os.sep + "Replay" + os.sep
    replay_names = os.listdir(replay_dir)
    for replay_name in replay_names:
        if not replay_name.endswith(".json"):
            replay_names.remove(replay_name)
    if len(replay_names) == 0:
        raise Exception("No Replay File in default dir")

    def TimeStampToTime(timestamp):
        timeStruct = time.localtime(timestamp)
        return timeStruct

    def get_FileModifyTime(filePath):
        t = os.path.getmtime(filePath)
        return TimeStampToTime(t)

    newest_t = get_FileModifyTime(replay_dir + replay_names[0])
    name = replay_names[0]

    for replay_name in replay_names[1:]:
        T = get_FileModifyTime(replay_dir + replay_name)
        if newest_t < T:
            newest_t = T
            name = replay_name


    if sys.platform.startswith('win32'):
        unity_game_dir = ".." + os.sep + "arena9" + os.sep + "Game" + os.sep + "Doto" + os.sep + "Doto.exe"
        os.system(unity_game_dir + " --replay " + replay_dir + name)
        pass
    elif sys.platform.startswith('linux'):
        #TODO Linux Code
        pass