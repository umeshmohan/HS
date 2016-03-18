#ifndef __BlobParam_included__
#define __BlobParam_included__

#include "common.hpp"

struct BlobParams
{
    cv::SimpleBlobDetector::Params params;
    vector<cv::KeyPoint> blobs_to_track;
    cv::Ptr<cv::SimpleBlobDetector> detector;
};

extern BlobParams GetBlobParams(cv::Mat input_image);

#endif
