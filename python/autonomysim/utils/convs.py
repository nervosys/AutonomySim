import math
import numpy as np
# import sys
# import os
# import inspect
# import re
# import logging
# import types
# import time

from autonomysim.types import Quaternionr
from autonomysim.utils import get_public_fields


def string_to_uint8_array(bstr):
    return np.fromstring(bstr, np.uint8)


def string_to_float_array(bstr):
    return np.fromstring(bstr, np.float32)


def list_to_2d_float_array(flst, width, height):
    return np.reshape(np.asarray(flst, np.float32), (height, width))


def to_dict(obj):
    return dict([attr, getattr(obj, attr)] for attr in get_public_fields(obj))


def to_str(obj):
    return str(to_dict(obj))


# helper method for converting getOrientation to roll/pitch/yaw
# https:#en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles


def to_eularian_angles(q):
    z = q.z_val
    y = q.y_val
    x = q.x_val
    w = q.w_val
    ysqr = y * y

    # roll (x-axis rotation)
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + ysqr)
    roll = math.atan2(t0, t1)

    # pitch (y-axis rotation)
    t2 = +2.0 * (w * y - z * x)
    if t2 > 1.0:
        t2 = 1
    if t2 < -1.0:
        t2 = -1.0
    pitch = math.asin(t2)

    # yaw (z-axis rotation)
    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (ysqr + z * z)
    yaw = math.atan2(t3, t4)

    return (pitch, roll, yaw)


def to_quaternion(pitch, roll, yaw):
    t0 = math.cos(yaw * 0.5)
    t1 = math.sin(yaw * 0.5)
    t2 = math.cos(roll * 0.5)
    t3 = math.sin(roll * 0.5)
    t4 = math.cos(pitch * 0.5)
    t5 = math.sin(pitch * 0.5)

    q = Quaternionr()
    q.w_val = t0 * t2 * t4 + t1 * t3 * t5  # w
    q.x_val = t0 * t3 * t4 - t1 * t2 * t5  # x
    q.y_val = t0 * t2 * t5 + t1 * t3 * t4  # y
    q.z_val = t1 * t2 * t4 - t0 * t3 * t5  # z
    return q
