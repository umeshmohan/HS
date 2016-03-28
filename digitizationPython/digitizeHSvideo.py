#!/usr/bin/env python3

from subprocess import Popen, PIPE
from os import devnull, path
import numpy as np
from opencvLinuxPythonBinary import cv2
import cairo

script_dir = path.dirname(path.realpath(__file__))
ffmpeg = path.join(script_dir, "ffmpeg")

def getVideoInfo(file_path):
    temp=Popen([ffmpeg, '-i', file_path], stdout=PIPE, stderr=PIPE)
    stdout, stderr = temp.communicate()
    stream0_info = stderr.decode('UTF-8')
                   .split("Stream #")[1].split("\n")[0].split(", ")
    fps = None
    for info in stream0_info:
        if info.count('x') == 1:
            [width, height] = [int(x) for x in info.split('x')]
        if info.count('fps') == 1:
            fps = int(info.split(" fps")[0])
        if info.count('tbr') == 1:
            tbr = int(float(info.split(" tbr")[0]
                            .replace('M', 'e6').replace('k', 'e3')))
    if fps is None:
        fps = tbr
    return width, height, fps

def ffmpegReadPipe(file_path):
    ffmpeg_command = [ffmpeg,
                      '-i', file_path,
                      '-f', 'image2pipe',
                      '-pix_fmt', 'gray16le',
                      '-vcodec', 'rawvideo',
                      '-threads', '0',
                      '-']
    dev_null = open(devnull, 'w')
    return Popen(ffmpeg_command, stdout=PIPE, stderr=dev_null)

def getFrame(ffmpeg_pipe, width, height):
    return np.fromstring(ffmpeg_pipe.stdout.read(width*height*2), 
                         dtype=np.uint16)

def ffmpegWritePipe(file_path, width, height):
    ffmpeg_command = ['ffmpeg', '-y',
                      '-r', '20',
                      '-f', 'image2pipe',
                      '-vcodec', 'png',
                      '-s', str(width)+'x'+str(height),
                      '-pix_fmt','rgba',
                      '-i', '-',
                      '-vcodec', 'libx264',
                      '-crf', '23',
                      '-pix_fmt', 'yuv420p',
                      '-an',
                      '-threads', '0',
                      file_path]
    return Popen(ffmpeg_command, stdin=PIPE)

def putFrame(destination, frame):
    if type(destination) is Popen:
        frame.write_to_png(destination.stdin)
    else:
        frame.write_to_png(destination)

def distanceFromBottomCenter(point, width, height):
    return sqrt(pow(point[0] - (width/2), 2) + 
                pow(point[1] - (height), 2))

def rearrangePoints(point_list, width, height):
    points_dict = {}
    for i in range(4):
        current_point = [point_list[i*2], point_list[(i*2)+1]]
        points_dict.update({distanceFromBottomCenter(
                                current_point, width, height):current_point})
    distance_list = points_dict.keys()
    distance_list.sort()
    rearranged_points_list = []
    for distance in distance_list:
        rearranged_points_list += points_dict[distance]
    return rearranged_points_list

def markImage(frame_data, *argv):
    img = cairo.ImageSurface.create_for_data(frame_data,
        cairo.FORMAT_RGB24, width, height, width*4)
    c = cairo.Context(img)
    c.select_font_face('UbuntuMono', 
                       cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
    c.set_font_size(25)
    for marking in argv:
        c.set_source_rgba(marking['color'][0],
                          marking['color'][1],
                          marking['color'][2],
                          marking['color'][3])
        if 'from_to' in marking:
            c.set_line_width(marking['width'])
            marking_from_to = lineEndPoints(marking['from_to'], 
                                           (img.get_width(), img.get_height()))
            c.move_to(marking_from_to[0], marking_from_to[1])
            c.line_to(marking_from_to[2], marking_from_to[3])
            c.stroke()
        if 'legend_text' in marking:
            c.move_to(marking['legend_position'][0], 
                      marking['legend_position'][1])
            c.show_text(marking['legend_text'])
    return img

def lineEndPoints(point_list, size):
    width, height = size
    x1, y1, x2, y2 = point_list
    if x1 == x2:
        return [x1, 0, x1, height]
    if y1 == y2:
        return [0, y1, width, y1]
    m = float(y1 - y2) / (x1 - x2)
    c = y1 - (m * x1)
    y_start = 0
    y_end = height
    x_start = -c / m
    if x_start < 0:
        x_start = 0
        y_start = c
    elif x_start > width:
        x_start = width
        y_start = (m * x_start) + c
    x_end = (height - c) / m
    if x_end < 0:
        x_end = 0
        y_end = c
    elif x_end > width:
        x_end = width
        y_end = (m * x_end) + c
    return [x_start, y_start, x_end, y_end]
