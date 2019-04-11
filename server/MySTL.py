import math
from math import sqrt, fabs, atan2
from BaseClass import *
from Arguments import room_size
eps = 1e-2


def L2Distance(p1, p2):
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y))


def PointToCoordinate(p):
    return int(p.x / room_size), int(p.y / room_size)


def PointInRectangle(p, rect):
    return rect.left <= p.x and p.x <= rect.right and rect.bottom <= p.y and p.y <= rect.top


def Angle(p1, p2):
    if p1 == p2:
        raise Exception("Try to get Angle with illegal Argument")
    p = p2 - p1
    angle = atan2(p.y, p.x)
    return angle


def MoveAlongAngle(nowPos, angle, dis):

    dx = 1.0 * dis * math.cos(angle)
    dy = 1.0 * dis * math.sin(angle)

    return nowPos + Point(dx, dy)


def LegalPos(pos, walls):
    if not (0 <= pos.x and pos.x < width and 0 <= pos.y and pos.y < height):
        return False
    x, y = PointToCoordinate(pos)
    return not walls[x][y]
