import shlex
import subprocess
import threading
import time
import platform


BYTEORDER = 'big'
players = []
gameover = False


class judger (threading.Thread):
    def __init__(self, cmd):
        threading.Thread.__init__(self)
        self.subpro = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE,
                                       stdin=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)

    def write(self, msg):
        print("write to judger")
        self.subpro.stdin.buffer.write(msg)
        self.subpro.stdin.flush()

    def run(self):
        error = int('1').to_bytes(4, byteorder=BYTEORDER, signed=True)
        t = self.subpro.stdout.buffer
        while True:
            Len = int.from_bytes(t.read(4), byteorder=BYTEORDER, signed=True)
            Type = int.from_bytes(t.read(4), byteorder=BYTEORDER, signed=True)
            if Type == 0:
                print("error: {}".format(str(error)))
                UserCode = int.from_bytes(
                    t.read(4), byteorder=BYTEORDER, signed=True)
                print("len : {} type: {} usercode: {}".format(Len, Type, UserCode))
                Len -= 8
                data = t.read(Len)
                tosend = Len.to_bytes(4, byteorder=BYTEORDER, signed=True)
                tosend += data
                print("read from judger:")
                print(data)
                if Len != 28:
                    if UserCode == -1:
                        for player in players:
                            player.write(tosend)
                    else:
                        players[UserCode].write(tosend)
                else:
                    print("don't send")
            elif Type == 2:
                global gameover
                gameover = True
                Len -= 4
                data = t.read(Len)
                print("len : {} type: {}".format(Len, Type))
                print("data :{}".format(data))
                tosend = Len.to_bytes(4, byteorder=BYTEORDER, signed=True)
                tosend += data
                for player in players:
                    player.write(tosend)
                time.sleep(5)
                data = t.read(600)
                print(data)
                break
            else:
                error += Len.to_bytes(4, byteorder=BYTEORDER, signed=True)
                error += Type.to_bytes(4, byteorder=BYTEORDER, signed=True)
        print("end judger")


if platform.system() == "Windows":
    jud = judger("python main.py")
else:
    jud = judger("python3 main.py")
print("start judger")


class player (threading.Thread):
    def __init__(self, cmd, usercode):
        threading.Thread.__init__(self)
        self.subpro = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE,
                                       stdin=subprocess.PIPE, stderr=subprocess.STDOUT,
                                       universal_newlines=True)
        self.usercode = usercode

    def write(self, msg):
        print("write to player{}".format(self.usercode))
        self.subpro.stdin.buffer.write(msg)
        self.subpro.stdin.flush()

    def run(self):
        t = self.subpro.stdout.buffer
        global gameover
        while True:
            #        Data = t.read(300)    #正常使用请注调这两行
            #        print("player data: {}".format(Data))      #如上
            Len = int.from_bytes(t.read(4), byteorder=BYTEORDER, signed=True)
            print("player Len: {}".format(Len))
            data = t.read(Len)
            Len += 8
            type = 0
            tosend = Len.to_bytes(4, byteorder=BYTEORDER, signed=True)
            tosend += type.to_bytes(4, byteorder=BYTEORDER, signed=True)
            tosend += self.usercode.to_bytes(4,
                                             byteorder=BYTEORDER, signed=True)
            tosend += data
            print("read from player:")
            print(data)
            jud.write(tosend)
        print("end player{}".format(self.usercode))


# 请在双引号内输入命令
if platform.system() == "Windows":
    players.append(player("mainbase.exe", 0))
    players.append(player("main0.exe", 1))
else:
    players.append(player("./mainbase.exe", 0))
    players.append(player("./main0.exe", 1))
print("start player")
for pla in players:
    pla.start()
jud.start()
print("finish start")
