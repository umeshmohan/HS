from subprocess import Popen, PIPE
from os import devnull, path
import numpy as np

script_dir = path.dirname(path.realpath(__file__))
ffmpeg = path.join(script_dir, "ffmpeg")

if not path.isfile(ffmpeg):
    from updateffmpeg import updateffmpeg
    updateffmpeg()

class EndOfVideoError(Exception):
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return repr(self.msg)

class CineFileClosedError(Exception):
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return repr(self.msg)

class readCine:
    def __init__(self, file_path):
        [self.width, self.height, 
         self.frame_rate, self.total_frames] = getVideoInfo(file_path)
        self._file_path = file_path
        self._ffmpeg_read_pipe = ffmpegReadPipe(file_path)
        self.is_open = True
        self.current_frame_number = -1
        self.current_frame = None
        self.getNextFrame()

    def getCurrentFrame(self):
        if self.is_open:
            return self.current_frame_number, self.current_frame
        else:
            raise CineFileClosedError("Cine file closed. Cannot read frame") 

    def getNextFrame(self):
        if not self.is_open:
            raise CineFileClosedError("Cine file closed. No next frame to read")
        if self.current_frame_number == self.total_frames -1:
            raise EndOfVideoError("Reached end of video") 
        else:
            self.current_frame_number += 1
            self.current_frame = getFrame(self._ffmpeg_read_pipe, 
                                          self.width, self.height)
            return self.getCurrentFrame()
    
    def getFrame(self, frame_number):
        if not self.is_open:
            raise CineFileClosedError("Cine file closed. Cannot read frame")
        if frame_number >= self.total_frames:
            raise EndOfVideoError("Invalid frame number: " + 
                                  str(frame_number) + 
                                  "\nMaximum frames in this video is " + 
                                  str(self.total_frames))
        else:
            if frame_number < self.current_frame_number:
                self.__init__(self.file_path)
            for i in range(frame_number):
                t = self.getNextFrame()
            return self.getCurrentFrame()

    def close(self):
        self._ffmpeg_read_pipe.terminate()
        self.is_open = False
        self.current_frame = None

def getVideoInfo(file_path):
    temp=Popen([ffmpeg, '-i', file_path], stdout=PIPE, stderr=PIPE)
    stdout, stderr = temp.communicate()
    stream0_info = stderr.decode('UTF-8')\
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
    duration = stderr.decode('UTF-8').split("Duration: ")[1].split(",")[0]
    duration = duration.split(':')
    duration = (int(duration[0]) * 60 * 60) + \
               (int(duration[1]) * 60) + \
               float(duration[2])
    total_frames = int(duration * fps)
    return width, height, fps, total_frames

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

