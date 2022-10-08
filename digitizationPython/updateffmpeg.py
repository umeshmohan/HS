#!/usr/bin/env python3

from urllib.request import urlretrieve
from os import path, listdir
from shutil import move, rmtree
import tarfile
from hashlib import md5
 
ffmpeg_linux_URL = "http://johnvansickle.com/ffmpeg/builds/ffmpeg-git-64bit-static.tar.xz"
ffmpeg_linux_file_name = ".ffmpeg-git-64bit-static.tar.xz"
script_dir = path.dirname(path.realpath(__file__))

def md5compare(file_path, file_md5_path):
    with open(file_md5_path, 'r') as md5_file:
        md5_from_file = md5_file.readline().split(" ")[0]
    if path.isfile(file_path):
        md5hash = md5()
        with  open(file_path, 'rb') as file_handle:
            for chunk in iter(lambda: file_handle.read(4096), b""):
                md5hash.update(chunk)
    else:
        return False
    md5_of_file = md5hash.hexdigest()
    return md5_of_file == md5_from_file

def downloadLatest(URL, file_name):
    print("\n" + file_name)
    urlretrieve(URL + ".md5", file_name + ".md5")
    print("got latest md5 of " + file_name)
    print("Computing md5 of local " + file_name)
    if md5compare(path.join(script_dir, file_name), 
                  path.join(script_dir, file_name + ".md5")):
        print("md5 of local and latest " + file_name + " match")
    else:
        print("md5 of local and latest " + file_name + " do not match")
        print("downloading latest " + file_name)
        urlretrieve(URL, file_name)

def extractffmpeg():
    print("Extracting ffmpeg from archive")
    with tarfile.open(ffmpeg_linux_file_name,'r:xz') as ffmpeg_tar_file:
        
        import os
        
        def is_within_directory(directory, target):
            
            abs_directory = os.path.abspath(directory)
            abs_target = os.path.abspath(target)
        
            prefix = os.path.commonprefix([abs_directory, abs_target])
            
            return prefix == abs_directory
        
        def safe_extract(tar, path=".", members=None, *, numeric_owner=False):
        
            for member in tar.getmembers():
                member_path = os.path.join(path, member.name)
                if not is_within_directory(path, member_path):
                    raise Exception("Attempted Path Traversal in Tar File")
        
            tar.extractall(path, members, numeric_owner=numeric_owner) 
            
        
        safe_extract(ffmpeg_tar_file)
    ffmpeg_dir = None
    for dir_content in listdir("."):
        if path.isdir(path.join(".", dir_content)) and dir_content.startswith("ffmpeg"):
            ffmpeg_dir = dir_content
    move(path.join(script_dir,ffmpeg_dir, "ffmpeg"),
         path.join(script_dir, "ffmpeg"))
    rmtree(path.join(script_dir, ffmpeg_dir))
    print("Done")

def updateffmpeg():
    downloadLatest(ffmpeg_linux_URL, ffmpeg_linux_file_name)
    extractffmpeg()
    
if __name__ == "__main__":
    updateffmpeg()
    print()
