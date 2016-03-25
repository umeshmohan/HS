#! /usr/bin/env python
# -*- coding: utf-8 -*-
#sed -e '/^#/d' in.csv >> in_comments_removed.csv

from os import devnull, path, makedirs
import argparse
import re
from subprocess import Popen, PIPE
import numpy as np
import cairo


def makeDir(path_name):
    if not path.exists(path_name):
        makedirs(path_name)

def ffmpegReadPipe(file_path):
    temp=Popen(['ffmpeg', '-i', file_path], stdout=PIPE, stderr=PIPE)
    stdout, stderr = temp.communicate()
    pattern = re.compile(r'([1-9]\d+)x(\d+)')
    match = pattern.search(stderr)
    width = int(match.groups()[0])
    height = int(match.groups()[1])
    ffmpeg_command = ['ffmpeg',
                      '-i', file_path,
                      '-f', 'image2pipe',
                      '-pix_fmt', 'rgba',
                      '-vcodec', 'rawvideo',
                      '-threads', '0',
                      '-']
    dev_null = open(devnull, 'w')
    return Popen(ffmpeg_command, stdout=PIPE, stderr=dev_null), \
           width, height

def ffmpegWritePipe(file_path, width, height):
    ffmpeg_command = ['ffmpeg', '-y',
                      '-r', '5',
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

def getFrame(ffmpeg_pipe, width, height):
    return cairo.ImageSurface.create_for_data(np.fromstring(
        ffmpeg_pipe.stdout.read(width*height*4), dtype=np.uint8),
        cairo.FORMAT_RGB24, width, height, width*4)

def putFrame(destination, frame):
    if type(destination) is Popen:
        frame.write_to_png(destination.stdin)
    else:
        frame.write_to_png(destination)
    

def markImage(img, *argv):
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

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Draw head and thorax\
            line on digitized head stabilization video with digitized \
            data.")
    parser.add_argument('-v', '--video', help="Video file", 
                        required=True, dest='video_file')
    parser.add_argument('-d', '--data', help="Processed digitized data file",
                        required=True, dest='data_file')
    parser.add_argument('-o', '--output', help="Output annotated video file",
                        required=False, dest='output_file',
                        default=None)
    parser.add_argument('-w', '--width', help="Line Width",
                        required=False, dest='line_width',
                        type=int, default=4)
    parser.add_argument('-f', '--fps', help="Video frames per second",
                        required=False, dest='fps',
                        type=int, default=3000)
    parser.add_argument('-i', '--imageseq', 
                        help="Output image sequence instead of video",
                        action='store_true')
    
    args = parser.parse_args()
    video_file = args.video_file
    data_file = args.data_file
    output_file = args.output_file
    line_width = args.line_width
    fps = args.fps

    input_video_pipe, width, height = ffmpegReadPipe(video_file)
    data = np.genfromtxt(data_file, delimiter=',', skip_header=1)
    number_of_frames = data.shape[0]
    print number_of_frames
    if output_file is None:
        output_file = video_file[:-4] + "-marked.mp4"
    if args.imageseq:
        output_dir = video_file[:-4] + "marked"
        print output_dir
        makeDir(output_dir)
    else:
        output_video_pipe = ffmpegWritePipe(output_file, width, height)

    for frame_number in range(number_of_frames):
        time = "  Time : " + '%3.4f' % (frame_number / float(fps)) + "s"
        frame_read = getFrame(input_video_pipe, width, height)
        head_points = data[frame_number][1:5]
        thorax_points = data[frame_number][5:9]
        thorax_angle = -np.rad2deg(np.arctan2(thorax_points[0]-thorax_points[2],
                                              thorax_points[1]-thorax_points[3]))
        head_angle = -np.rad2deg(np.arctan2(head_points[0]-head_points[2], 
                                            head_points[1]-head_points[3]))
        thorax_legend = "Thorax : " + '%+03.1f' % (thorax_angle) + u"°"
        head_legend = "  Head : " + '%+03.1f' % (head_angle) + u"°"
        marked_frame  = markImage(frame_read,
                                  {'from_to':thorax_points,
                                   'color':(1,0,0,0.5),
                                   'width':line_width,
                                   'legend_text':thorax_legend,
                                   'legend_position':(20,20),
                                   },
                                  {'from_to':head_points,
                                   'color':(1,1,0,0.5),
                                   'width':line_width,
                                   'legend_text':head_legend,
                                   'legend_position':(20,50)
                                   },
                                  {'legend_text':time,
                                   'legend_position':(20,700),
                                   'color':(1,1,1,0.5)})
        if args.imageseq:
            putFrame(output_dir + "/%05d.png" % frame_number, marked_frame)
        else:
            putFrame(output_video_pipe, marked_frame)

    input_video_pipe.terminate()
    output_video_pipe.terminate()

# vim: set ai nu et ts=4 sw=4:
