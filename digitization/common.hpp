#ifndef __Common_included__
#define __Common_included__

#include "cmdline.h"
#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

namespace ffmpeg {
    extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    }
}

using namespace std;

extern float MAX_DISTANCE, DISPLAY_SCALE;
extern int BLOB_COLOR;

#endif
