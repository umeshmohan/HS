#!/bin/bash
input_file=`yad --file --width=800 --height=600 --title="Select avi file"`
if (($? == 0)); then
    ffmpeg -i "$input_file" -q:v 2 -vcodec wmv2 -threads 0 -pix_fmt yuv420p "${input_file%%avi}wmv"
fi
