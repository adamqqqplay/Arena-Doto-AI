PYGAME = False
DEBUG = False

if PYGAME:
    import contextlib
    with contextlib.redirect_stdout(None):
        import pygame
import sys
import random
import copy
import time
import datetime
import threading
from Arguments import *
from BaseClass import *
from MySTL import *
from multiprocessing import Pool
import json
import logging
import os

BYTEORDER = 'big'

test_num = random.randint(0, 9999)

fireball_no = 0
meteor_no = 0

save_dir = None

if DEBUG:
    logfile_dir = "." + os.sep + "DEBUG" + os.sep
    if not os.path.exists(logfile_dir):
        os.makedirs(logfile_dir)

    logfile_name = logfile_dir + "logfile{}.txt".format(test_num)

    def WriteToLogFile(*ArgTuple, end='\n', sep=' '):
        with open(logfile_name, "a") as f:
            for Arg in ArgTuple[:-1]:
                f.write(str(Arg))
                f.write(sep)
            if len(ArgTuple) > 0:
                f.write(str(ArgTuple[-1]))
            f.write(end)

if PYGAME:
    pygame.init()
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption("Defense Of The sOgou")

    def draw_wall(x, y):
        pygame.draw.rect(screen, black, pygame.Rect(x * room_size, y * room_size,
                                                    room_size, room_size))

    def draw_ball(ball):
        pygame.draw.circle(screen, green, (int(ball.pos.x),
                                           int(ball.pos.y)), int(ball.radius))

    def draw_fireball(fireball):
        pygame.draw.circle(screen, red, (int(fireball.pos.x),
                                         int(fireball.pos.y)), int(fireball.radius))

    def draw_target(target):
        pygame.draw.circle(screen, gray, (int(target.pos.x),
                                          int(target.pos.y)), int(target.radius))
    def draw_bonus(bonus):
        pygame.draw.circle(screen, yellow, (int(bonus.pos.x), int(bonus.pos.y)), int(bonus.radius))


    def draw_meteor(meteor):
        pygame.draw.circle(screen, pink, (int(meteor.pos.x), int(
            meteor.pos.y)), int(meteor.attack_radius))

    def draw_human(human):
        if human.inv_time > 0:
            pygame.draw.circle(screen, golden, (int(human.pos.x), int(
                human.pos.y)), int(fireball_radius + 3))
        pygame.draw.circle(screen, blue, (int(human.pos.x),
                                          int(human.pos.y)), int(fireball_radius))
        myfont = pygame.font.Font(None, 20)
        textImage = myfont.render(str(human.hp), True, blue)
        screen.blit(textImage, (human.pos.x, human.pos.y))

    def draw_all(humans, walls, balls, fireballs, meteors, targets , bonuses):
        screen.fill(white)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                sys.exit()
        for target in targets:
            draw_target(target)
        for meteor in meteors:
            draw_meteor(meteor)
        for x in range(int(width / room_size)):
            for y in range(int(height / room_size)):
                if walls[x][y]:
                    draw_wall(x, y)
        for human in humans:
            if human.death_time == -1:
                draw_human(human)
        for fireball in fireballs:
            draw_fireball(fireball)
        for ball in balls:
            draw_ball(ball)
        for bonus in bonuses:
            if bonus.time==-1:
                draw_bonus(bonus)
        pygame.display.flip()

events = []
logs = []
humans = [None] * faction_number * human_number
balls = [None] * faction_number
fireballs = []
meteors = []
targets = [TargetArea(Point(t[0], t[1])) for t in target_places]
bonuses = [Bonus(num,Point(t[0],t[1])) for num,t in enumerate(bonus_places)]
score = [0.0] * faction_number

debugMsgs = ['']*faction_number

def sendLog(log, Type=0, UserCode=-1):
    if DEBUG:
        WriteToLogFile("~~~~~~Send Msg(Real time = {})~~~~~~".format(
            datetime.datetime.now().strftime('%H:%M:%S.%f')))
        WriteToLogFile("Type=", Type, "UserCode", UserCode)
        WriteToLogFile(log)
        WriteToLogFile("~~~~~~                        ~~~~~~")
    Body = json.dumps(log).encode()
    if Type == 2:
        Len = len(Body) + 4
        toSend = Len.to_bytes(4, byteorder=BYTEORDER, signed=True)
        toSend += Type.to_bytes(4, byteorder=BYTEORDER, signed=True)
        toSend += Body
        sys.stdout.buffer.write(toSend)
        sys.stdout.flush()
    else:
        Len = len(Body) + 8
        toSend = Len.to_bytes(4, byteorder=BYTEORDER, signed=True)
        toSend += Type.to_bytes(4, byteorder=BYTEORDER, signed=True)
        toSend += UserCode.to_bytes(4, byteorder=BYTEORDER, signed=True)
        toSend += Body
        sys.stdout.buffer.write(toSend)
        sys.stdout.flush()


def LegalInfo(data):
    try:
        assert isinstance(data, dict)
        assert "flag" in data
        if data["flag"] == 1 or data["flag"] == 2:
            return True
        assert data["flag"] == 0
        assert "move" in data and "shoot" in data and "meteor" in data and "flash" in data
        assert isinstance(data["flash"], list)
        assert len(data["flash"]) == human_number
        for b in data["flash"]:
            assert b == True or b == False

        def check(l):
            assert isinstance(l, list)
            assert len(l) == human_number
            for item in l:
                assert isinstance(item, list)
                assert len(item) == 2
                for i in item:
                    assert isinstance(i, float)or isinstance(i, int)
        check(data["move"])
        check(data["shoot"])
        check(data["meteor"])
    except:
        return False
    else:
        return True


class Listen(threading.Thread):
    null = {
        "flag": 1,
        "move": [[-1, -1]] * human_number,
        "shoot": [[-1, -1]] * human_number,
        "meteor": [[-1, -1]] * human_number,
        "flash": [False] * human_number,
        "debug":"",
    }

    def __init__(self):
        super(Listen, self).__init__()
        self.ans = []
        for i in range(faction_number):
            self.ans.append(self.null)

    def recvData(self):
        Len = int.from_bytes(sys.stdin.buffer.read(
            4), byteorder=BYTEORDER, signed=True)
        Type = int.from_bytes(sys.stdin.buffer.read(
            4), byteorder=BYTEORDER, signed=True)
        UserCode = int.from_bytes(sys.stdin.buffer.read(
            4), byteorder=BYTEORDER, signed=True)
        if Type == 0:  # 用户AI发送的包
            data = str(sys.stdin.buffer.read(Len - 8), 'utf-8')
            try:
                JSON = json.loads(data)  # 读取主体部分
            except:
                if DEBUG:
                    WriteToLogFile("~~~~~~Recv Bad Msg(Real time = {})~~~~~~".format(
                        datetime.datetime.now().strftime('%H:%M:%S.%f')))
                    WriteToLogFile('AI:', UserCode)
                    WriteToLogFile(data)
                    WriteToLogFile("~~~~~~                        ~~~~~~")
                return True

            if LegalInfo(JSON):
                if DEBUG:
                    WriteToLogFile("~~~~~~Recv Msg(Real time = {})~~~~~~".format(
                        datetime.datetime.now().strftime('%H:%M:%S.%f')))
                    WriteToLogFile('AI:', UserCode)
                    WriteToLogFile(data)
                    WriteToLogFile("~~~~~~                        ~~~~~~")
                if JSON["flag"] == 0:
                    self.ans[UserCode] = JSON
                    return True
                elif JSON["flag"] == 1:
                    self.ans[UserCode] = null
                    return True
                elif JSON["flag"] == 2:
                    return False
            else:
                if DEBUG:
                    WriteToLogFile("~~~~~~Recv BAD MSG(Real time = {})~~~~~~".format(
                        datetime.datetime.now().strftime('%H:%M:%S.%f')))
                    WriteToLogFile('AI:', UserCode)
                    WriteToLogFile(data)
                    WriteToLogFile("~~~~~~                        ~~~~~~")
                return True

            if JSON["flag"] == 0:
                if LegalInfo(JSON):
                    self.ans[UserCode] = JSON
                else:
                    self.ans[UserCode] = null
                return True
            elif JSON["flag"] == 1:
                self.ans[UserCode] = null
                return True
            elif JSON["flag"] == 2:
                return False

    def run(self):
        while True:
            if not self.recvData():
                break


def Ev(*Args):
    lt = []
    for Arg in Args:
        lt.append(Arg)

    if DEBUG:
        if lt[0] == 1:
            WriteToLogFile("Player {} shoots!".format(lt[1]))
        elif lt[0] == 2:
            WriteToLogFile("Player {} gets {} hurt from {}!".format(
                lt[1], lt[2], lt[3]))
        elif lt[0] == 3:
            WriteToLogFile("Player {} died at ({},{})!".format(
                lt[1], lt[2], lt[3]))
        elif lt[0] == 4:
            WriteToLogFile("Player {} cast Meteor!".format(
                lt[1]))
        elif lt[0] == 5:
            WriteToLogFile("Player {} gets ball!".format(
                lt[1]))
        elif lt[0] == 6:
            WriteToLogFile("A Fireball from {} splashes at ({},{})!".format(
                lt[3], lt[1], lt[2]))
        elif lt[0] == 7:
            WriteToLogFile("A Meteor from {} impacts at ({},{})!".format(
                lt[3], lt[1], lt[2]))
        elif lt[0] == 8:
            WriteToLogFile("Player {} reincarnate!".format(
                lt[1]))
        elif lt[0] == 9:
            WriteToLogFile("Player {} flashes from ({},{}) to ({},{})!".format(
                lt[1], lt[2], lt[3], lt[4], lt[5]))
        elif lt[0] == 10:
            WriteToLogFile("Player {} goals with faction{}'s ball!".format(
                lt[1], lt[2]))
        elif lt[0] == 11:
            WriteToLogFile("Bonus_place {} generate a bonus!".format(lt[1]))
        elif lt[0] == 12:
            WriteToLogFile("Player {} get a bonus at bonus_place {}".format(lt[1],lt[2]))
    events.append(lt)


def EvClear():
    global events
    events.clear()


def init():
    log = {
        "frame": 0,
        "map": map_id,
    }
    logs.append(copy.deepcopy(log))
    for i in range(faction_number):
        log["faction"] = i
        sendLog(log, 0, i)

    for fac, birth_place in enumerate(birth_places):
        for i, pos in enumerate(birth_place):
            humans[i * faction_number +
                   fac] = Human(i * faction_number + fac, Point(pos[0], pos[1]))

    for fac, ball_place in enumerate(ball_places):
        balls[fac] = Ball(Point(ball_place[0], ball_place[1]), fac)
        balls[fac].reset()

    for bonus in bonuses:
        bonus.reset()


def flash(human, pos):
    if human.death_time != -1 or human.flash_time > 0 or human.flash_number == 0:
        return
    for ball in balls:
        if ball.belong == human.number:
            return

    pos = Point(pos[0], pos[1])
    if L2Distance(human.pos, pos) <= eps + flash_distance and LegalPos(pos, walls):
        Ev(9, human.number, round(human.pos.x,precision), round(human.pos.y,precision), round(pos.x,precision), round(pos.y,precision))
        human.pos = pos
        human.flash_time = human.flash_interval
        human.flash_number -= 1


def move(human, pos):
    if human.death_time !=-1:
        return
    pos = Point(pos[0], pos[1])
    x, y = PointToCoordinate(pos)
    if L2Distance(human.pos, pos) <= eps + human_velocity and LegalPos(pos, walls):
        human.pos = pos
        if DEBUG:
            WriteToLogFile("human {} moves to {}".format(human.number,pos))


def fireball_hurt(fireball, human, hurt_record):
    if human.death_time != -1 or human.inv_time > 0:
        return
    if not friendly_fire:
        if fireball.from_number % faction_number == human.faction:
            return
    if L2Distance(fireball.pos, human.pos) <= fireball.attack_radius + eps:
        human.hp -= fireball.hurt
        if fireball.from_number in hurt_record[human.number]:
            hurt_record[human.number][fireball.from_number] += fireball.hurt
        else:
            hurt_record[human.number][fireball.from_number] = fireball.hurt
        Ev(2, human.number, fireball.hurt, fireball.from_number)


def meteor_hurt(meteor, human, hurt_record):
    if human.death_time != -1 or human.inv_time > 0:
        return
    if not friendly_fire:
        if meteor.from_number % faction_number == human.faction:
            return
    if L2Distance(meteor.pos, human.pos) <= meteor.attack_radius + eps:
        human.hp -= meteor.hurt
        if meteor.from_number in hurt_record[human.number]:
            hurt_record[human.number][meteor.from_number] += meteor.hurt
        else:
            hurt_record[human.number][meteor.from_number] = meteor.hurt
        Ev(2, human.number, meteor.hurt, meteor.from_number)


def death(human, hurt_dict):
    human.death_time = frames_of_death
    Ev(3, human.number, round(human.pos.x,precision), round(human.pos.y,precision))
    score[human.faction] += killed_score
    sum_hurt = 0
    for h_id, hurt in hurt_dict.items():
        sum_hurt += hurt
    for h_id, hurt in hurt_dict.items():
        score[h_id % faction_number] += 1.0 * kill_score * hurt / sum_hurt


def shoot(human, pos):
    if human.death_time != -1 or human.fireball_time > 0:
        return
    pos = Point(pos[0], pos[1])
    if human.pos.x == pos.x and human.pos.y == pos.y:
        return
    if not(0<=pos.x and pos.x<width and 0<=pos.y and pos.y<height):
        return
    ang = Angle(human.pos, pos)
    pos = MoveAlongAngle(human.pos, ang, splash_radius)
    global fireball_no
    if LegalPos(pos, walls):
        fireball_no+=1
        fireballs.append(Fireball(pos, ang, human.number,fireball_no))
        human.fireball_time = human.fireball_interval
        Ev(1, human.number, fireball_no)


def cast(human, pos):
    if human.death_time != -1 or human.meteor_time > 0 or human.meteor_number == 0:
        return
    pos = Point(pos[0], pos[1])
    if not PointInRectangle(pos, Rectangle(0, width, 0, height)):
        return
    global meteor_no
    if L2Distance(human.pos, pos) <= eps + meteor_distance:
        meteor_no += 1
        Ev(4, human.number,meteor_no)
        human.meteor_time = human.meteor_interval
        human.meteor_number -= 1
        meteors.append(Meteor(pos, human.number,meteor_no))


def pickupball(ball):
    mindis = 1e9
    num = -1
    for human in humans:
        if human.death_time == -1 and human.faction != ball.faction:
            Dis = L2Distance(human.pos, ball.pos)
            if Dis < mindis:
                mindis = Dis
                num = human.number
            elif Dis == mindis:
                if score[num % faction_number] < score[human.faction]:
                    num = human.number
    if mindis < ball.radius + eps:
        ball.belong = num
        ball.pos.x, ball.pos.y = humans[num].pos.x, humans[num].pos.y
        Ev(5, num)


def goal(ball):
    target = targets[ball.belong % faction_number]
    if L2Distance(ball.pos, target.pos) <= target.radius + eps:
        Ev(10, ball.belong, ball.faction)
        score[ball.belong % faction_number] += goal_score
        score[ball.faction] += goaled_score
        ball.reset()

def getbonus(bonus):
    mindis = 1e9
    num = -1
    for human in humans:
        if human.death_time==-1:
            Dis = L2Distance(human.pos,bonus.pos)
            if Dis < mindis:
                mindis = Dis
                num = human.number
            elif Dis == mindis:
                if score[num % faction_number] < score[human.faction]:
                    num = human.number
    if mindis < bonus.radius + eps:
        score[num%faction_number]+=bonus_score
        bonus.reset()
        Ev(12, num,bonus.number)


def RunGame():

    init()

    listener = Listen()
    listener.setDaemon(True)
    listener.start()

    time.sleep(1)

    timecnt = 0
    while timecnt < frames_of_game:
        timecnt += 1
        EvClear()
        if DEBUG:
            WriteToLogFile("--------------------------Time = {}(Real time = {})--------------------------".format(
                timecnt, datetime.datetime.now().strftime('%H:%M:%S.%f')))

        # Rebirth
        for human in humans:
            if human.death_time == 0:
                human.reset()
                human.death_time = -1
                Ev(8, human.number)
        if DEBUG:
            WriteToLogFile("Rebirth Succeed")

        # Reset Bonus
        for bonus in bonuses:
            if bonus.time == 0:
                bonus.time=-1
                Ev(11,bonus.number)
        if DEBUG:
            WriteToLogFile("Reset Bonus Succeed")


        # Send State
        log = {
            "frame": timecnt,
            "humans": str(humans),
            "fireballs": str(fireballs),
            "meteors": str(meteors),
            "balls": str(balls),
            "scores": str(score),
            "bonus":str(bonuses)
        }
        sendLog(log)

        if PYGAME:
            draw_all(humans, walls, balls, fireballs, meteors, targets,bonuses)

        if DEBUG:
            WriteToLogFile("Send Succeed")

        time.sleep(1.0 / frames_per_second)

        # listen
        analysis = copy.deepcopy(listener.ans)
        for i in range(faction_number):
            listener.ans[i] = listener.null
        if DEBUG:
            for i, an in enumerate(analysis):
                WriteToLogFile('Player {}:'.format(i), an)
        if DEBUG:
            WriteToLogFile("Listen Succeed")

        for fac,a in enumerate(analysis):
            if 'debug' in a:
                if a['debug']!='':
                    debugMsgs[fac]+='Frames = {}\n'.format(timecnt)
                    debugMsgs[fac]+=a['debug']
                    debugMsgs[fac]+='\n\n'

        # flash
        for fac, a in enumerate(analysis):
            for i, isflash in enumerate(a["flash"]):
                if isflash:
                    h_id = i * faction_number + fac
                    flash(humans[h_id], a["move"][i])
        if DEBUG:
            WriteToLogFile("Flash Succeed")

        # move human,fireball and ball
        delFireballs = []
        for fireball in fireballs:
            newpos = MoveAlongAngle(
                fireball.pos, fireball.rot, fireball.velocity)
            fireball.pos = newpos
            if not LegalPos(newpos, walls):
                delFireballs.append(fireball)

        for fac, a in enumerate(analysis):
            for i, pos in enumerate(a["move"]):
                h_id = i * faction_number + fac
                #WriteToLogFile("*** human[{}] goto"+str(pos))
                move(humans[h_id], pos)

        for ball in balls:
            if ball.belong != -1:
                ball.pos.x, ball.pos.y = humans[ball.belong].pos.x, humans[ball.belong].pos.y

        if DEBUG:
            WriteToLogFile("Move Succeed")

        # hurt

        for fireball in fireballs:
            if not(fireball in delFireballs):
                for human in humans:
                    if human.death_time==-1:
                        if not friendly_fire:
                            if human.faction == fireball.from_number % faction_number:
                                continue
                        if L2Distance(fireball.pos, human.pos) <= eps + fireball_radius and not(fireball in delFireballs):
                            delFireballs.append(fireball)

        delMeteors = []
        for meteor in meteors:
            if meteor.time == 0:
                delMeteors.append(meteor)

        hurt_record = [{}] * len(humans)
        for fireball in delFireballs:
            Ev(6, round(fireball.pos.x,precision), round(fireball.pos.y,precision), fireball.from_number)
            for human in humans:
                fireball_hurt(fireball, human, hurt_record)
            fireballs.remove(fireball)

        for meteor in delMeteors:
            Ev(7, round(meteor.pos.x,precision), round(meteor.pos.y,precision), meteor.from_number)
            for human in humans:
                meteor_hurt(meteor, human, hurt_record)
            meteors.remove(meteor)

        # death and kill score
        for human in humans:
            if human.hp <= 0 and human.death_time == -1:
                death(human, hurt_record[human.number])

        if DEBUG:
            WriteToLogFile("Hurt and Death Succeed")


        for bonus in bonuses:
            if bonus.time==-1:
                getbonus(bonus)

        # Cast
        for fac, a in enumerate(analysis):
            for i, pos in enumerate(a["meteor"]):
                h_id = i * faction_number + fac
                cast(humans[h_id], pos)
        if DEBUG:
            WriteToLogFile("Cast Succeed")

        # Shoot
        for fac, a in enumerate(analysis):
            for i, pos in enumerate(a["shoot"]):
                h_id = i * faction_number + fac
                shoot(humans[h_id], pos)
        if DEBUG:
            WriteToLogFile("Shoot Succeed")

        # Update balls
        for ball in balls:
            if humans[ball.belong].death_time != -1:
                ball.belong = -1

        for ball in balls:
            if ball.belong == -1:
                pickupball(ball)

        for ball in balls:
            if ball.belong != -1:
                goal(ball)

        if DEBUG:
            WriteToLogFile("Update balls Succeed")

        # Update all time
        for human in humans:
            if human.death_time == -1:
                if human.flash_time > 0:
                    human.flash_time -= 1
                if human.meteor_time > 0:
                    human.meteor_time -= 1
                if human.fireball_time > 0:
                    human.fireball_time -= 1
                if human.inv_time > 0:
                    human.inv_time -= 1
            else:
                if human.death_time > 0:
                    human.death_time -= 1

        for meteor in meteors:
            meteor.time -= 1

        for bonus in bonuses:
            if bonus.time > 0:
                bonus.time-=1

        if DEBUG:
            WriteToLogFile("Update Time Succeed")

        log = {
            "frame": timecnt,
            "humans": str(humans),
            "fireballs": str(fireballs),
            "meteors": str(meteors),
            "balls": str(balls),
            "scores": str(score),
            "bonus":str(bonuses),
            "events": str(events)
        }

        logs.append(log)
    log = {
        "frame": -1,
        "scores": str(score)
    }
    sendLog(log, 2, -1)
    logs.append(log)
    if save_dir == None:
        replay_dir = "." + os.sep + "Replay" + os.sep
        if not os.path.exists(replay_dir):
            os.mkdir(replay_dir)
        replay_dir += "replay{}.zip".format(test_num)
    else:
        replay_dir = save_dir
    name = replay_dir[replay_dir.rfind('/')+1:]
    Dir = replay_dir[:replay_dir.rfind('/')+1]
    for i in range(faction_number):
        with open(Dir+name.replace(".zip","_player{}_debug.txt".format(i)),'w')as file:
            file.write(debugMsgs[i])
    with open(Dir+name.replace(".zip",".json"), "w")as file:
        file.write(json.dumps(logs))
    command = "cd {0} && zip -rq {1} {2} ".format(Dir,name,name.replace(".zip",".json"))
    for i in range(faction_number):
        command += name.replace(".zip","_player{}_debug.txt".format(i))+" "
    command += "&& rm -f *.json *.txt"
    os.system(command)
    if DEBUG:
        WriteToLogFile("################### Result ###################")
        for i, sc in enumerate(score):
            WriteToLogFile(i, ":", sc)
        WriteToLogFile("###################        ###################")

    time.sleep(3)


if __name__ == "__main__":
    if len(sys.argv)==1:
        RunGame()
    else:
        if PYGAME or DEBUG:
            print("Please close PYGAME and DEBUG first!")
        else:
            save_dir = sys.argv[1]
            RunGame()
