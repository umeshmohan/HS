#! /usr/bin/env python
# -*- coding: utf-8 -*-
#sed -e '/^#/d' in.csv >> in_comments_removed.csv

import numpy as np
from math import sqrt, pow
from csv import writer as csvwriter
import argparse

def readDataCSV(file_name, width, height):
    data_dict = {}
    data = []
    with open(file_name, 'r') as file_handle:
        for line in file_handle:
            if line[0] != "#":
                line_contents = line.split(",")
                frame_number = int(line_contents[0])
                if frame_number not in data:
                    data_dict.update({frame_number:
                                [float(i) for i in line_contents[1:]]})
                else:
                    data_dict[frame_number] = [float(i) 
                                               for i in line_contents[1:]]
    for frame_number in data_dict.keys():
        data.append([frame_number] + rearrangePoints(data_dict[frame_number], 
                                                     width, height))
    return data

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

def resolution(s):
    try:
        w, h = map(int, s.split('x'))
        return w, h
    except:
        raise argparse.ArgumentTypeError(
                "Resolution must be <width>x<height>. E.g.: 1200x800")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Rearrange points in ascending order from bottom center of frame")
    parser.add_argument('-d', '--data', help="Input data file from digitization", 
                        required=True, dest='data_file')
    parser.add_argument('-o', '--output', help="Output rearranged data file", 
                        required=False, dest='output_file', default=None)
    parser.add_argument('-r', '--resolution', help="Frame resolution", 
                        required=False, dest='resolution', default=(1200,800), 
                        type=resolution)
    args = parser.parse_args()
    
    data = readDataCSV(args.data_file, args.resolution[0], args.resolution[1])
    if args.output_file is None:
        output_file = args.data_file[:-4] + ".rearranged.csv"
    else:
        output_file = args.output_file
    with open(output_file, 'w') as output_file_handle:
        out = csvwriter(output_file_handle, delimiter=',')
        out.writerows(data)

# vim: set ai nu et ts=4 sw=4:
