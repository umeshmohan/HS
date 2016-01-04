#ifndef __AutoTrack_included__
#define __AutoTrack_included__

#include "common.hpp"
#include "BlobParam.hpp"

class AutoTrackProgress
{
    public:
        vector<cv::KeyPoint> previous_frame_key_point_list;
        int MAX_frames;
        int analyzed_till;
        BlobParams blob_params;
        bool analysis_complete;
        AutoTrackProgress(BlobParams blob_params, int MAX_frames, 
                          int analyzed_till);
};

extern AutoTrackProgress auto_track_progress;

AutoTrackProgress AutoTrack(cv::VideoCapture video, 
    AutoTrackProgress auto_track_progress);

#endif
