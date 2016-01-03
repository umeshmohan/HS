# HeadStabilizationDigitization

## Setup

### OpenCV
1. Download latest OpenCV (https://github.com/Itseez/opencv/)
2. unzip
3. cd into the extracted dir
4. mkdir build
5. cmake-gui ..
6. make sure these are set
 * No dynamic libraries
 * Enable Qt5, Qt OpenGL support 
 * ffmpeg and gstreamer
 * python libraries
7. Preferably set install path to local accesible directory. (e.g. /home/opencv)
8. configure, quit CMake-gui
9. make -j\`nproc\`
10. make install

## Compiling
1. Create a build directory. (e.g. ../headStabilizationDigitization-bin)
2. cd into the bin directory and invoke cmake (cmake ../headStabilizationDigitization-src)
3. make

## using eclipse IDE
Complete steps 1 and 2 from the above section (compiling)

3. File -> New -> Makefile project with existing code
4. Select code location (headStabilizationDigitization-src)
5. Select toolchain as Linux GCC, Finish.
6. In Project -> Properties -> C/C++ Build, set Build directory to the directory from step 2 of compiling, Apply.
7. In Project -> Properties -> C/C++ General -> Paths and Symbols 
 1. -> Includes -> GNU C++, Add opencv include directory (/home/opencv/include), Apply.
 2. -> Library Paths, Add opencv lib directory (/home/opencv/lib), Apply.
8. OK.

