#include "AutoTrack.hpp"
#include "BlobParam.hpp"

AutoTrackProgress::AutoTrackProgress(BlobParams blob_paramsIN, 
                                     int MAX_framesIN, 
                                     int analyzed_tillIN, 
                          vector<cv::KeyPoint> previous_frame_key_point_listIN)
{
    MAX_frames = MAX_framesIN;
    analyzed_till = analyzed_tillIN;
    blob_params = blob_paramsIN;
    previous_frame_key_point_list = previous_frame_key_point_listIN;
}

float Distance(cv::Point a, cv::Point b)
{
    return sqrt(pow(a.x-b.x, 2) + pow(a.y-b.y, 2));
}

vector<cv::KeyPoint> GetBlobsToTrack(vector<cv::KeyPoint> frame_key_point_list, 
                            vector<cv::KeyPoint> previous_frame_key_point_list)
{
    int number_of_valid_blobs_found = 0;
    vector<cv::KeyPoint> valid_blobs(previous_frame_key_point_list.size());
    for (int i=0; i<previous_frame_key_point_list.size(); i++)
    {
        for (int j=0; j<frame_key_point_list.size(); j++)
        {
            if (Distance(previous_frame_key_point_list[i].pt, 
                         frame_key_point_list[j].pt) 
                <= previous_frame_key_point_list[i].size)
            {
                valid_blobs[number_of_valid_blobs_found] = 
                    frame_key_point_list[j];
                number_of_valid_blobs_found++;
                break;
            }
        }
    }
    vector<cv::KeyPoint> valid_blobs2(number_of_valid_blobs_found);
    for (int i=0; i<number_of_valid_blobs_found; i++)
    {
        valid_blobs2[i] = valid_blobs[i];
    }
    return valid_blobs2;
}

const char* WINDOW_AUTOTRACK = "Auto tracking";

AutoTrackProgress AutoTrack(cv::VideoCapture video, 
                            AutoTrackProgress auto_track_progress, 
                            vector<vector<cv::Point> > &detected_point_list)
{
    cv::Mat frame, frame_display;
    vector<cv::KeyPoint> frame_key_point_list, valid_blobs;
    video.set(cv::CAP_PROP_POS_FRAMES, auto_track_progress.analyzed_till - 1);
    auto_track_progress.analysis_complete = true; 
    while (auto_track_progress.analyzed_till + 1 
           < auto_track_progress.MAX_frames)
    {
        video.read(frame);
        auto_track_progress.blob_params.detector->detect(frame, 
                                                         frame_key_point_list);
        valid_blobs = GetBlobsToTrack(frame_key_point_list, 
                            auto_track_progress.previous_frame_key_point_list);
        if (valid_blobs.size() 
            != auto_track_progress.previous_frame_key_point_list.size())
        {
            cout << "# Blobs mismatch. Aborting auto tracking." << endl;
            auto_track_progress.analysis_complete = false;
            break;
        }
        else
        {
            auto_track_progress.analyzed_till++;
            cout << auto_track_progress.analyzed_till;
            for (int i=0; i<valid_blobs.size(); i++)
            {
                auto_track_progress.previous_frame_key_point_list[i] = 
                                                                valid_blobs[i];
                detected_point_list[auto_track_progress.analyzed_till][i] = 
                                                             valid_blobs[i].pt;
                cout << "," << valid_blobs[i].pt.x <<","<< valid_blobs[i].pt.y;
            }
            cout << endl;
            cv::drawKeypoints(frame, valid_blobs, frame_display, 
                              cv::Scalar(0, 255, 0), 
                              cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            cv::resize(frame_display, frame_display, cv::Size(), 0.7, 0.7);
            cv::imshow(WINDOW_AUTOTRACK, frame_display);
            stringstream message;
            message << "Analyzed " << auto_track_progress.analyzed_till << "/";
            message << auto_track_progress.MAX_frames - 1 << " frames";
            cv::displayStatusBar(WINDOW_AUTOTRACK, message.str(), 1000);
            cv::waitKey(10);
        }
    }
    cv::destroyAllWindows();
    return auto_track_progress;
}

