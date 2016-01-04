#include "BlobParam.hpp"
#include "AutoTrack.hpp"
#define USAGE "Usage:\nHSD <video_file> [start_from_frame_number(default=0)] [MAX_DISTANCE(default=0.5r)] [DISPLAY_SCALE(default=0.7)]"
using namespace std;

cv::Mat GetFrame(cv::VideoCapture video_in_capture, int frame_number)
{
    cv::Mat frame;
    video_in_capture.set(cv::CAP_PROP_POS_FRAMES, frame_number - 1);
    if(!video_in_capture.read(frame))
    {
        cerr << "Unable to read next frame. Exiting..." << endl;
        exit(EXIT_FAILURE);
    }
    return frame;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {cout << USAGE; return -1;}
    const string in_file_name = argv[1];
    int start_from;
    if (argc < 3) { start_from = 0; } else { start_from = atoi(argv[2]); }
    if (argc < 4) { MAX_DISTANCE = 0.5; } else { MAX_DISTANCE = atof(argv[3]); }
    if (argc < 4) { DISPLAY_SCALE = 0.7; } else { DISPLAY_SCALE = atof(argv[4]); }
    
    std::ios_base::openmode mode1 = ios_base::app; 
    ofstream out((in_file_name.substr(0,in_file_name.length()-3) 
                 + "csv").c_str(), mode1);
    std::cout.rdbuf(out.rdbuf());
    
    cv::VideoCapture video_in_capture(in_file_name);
    if (!video_in_capture.isOpened()) {
        cerr  << "Could not open video " << in_file_name << endl;
        exit(EXIT_FAILURE);
    }
    const int MAX_FRAMES = video_in_capture.get(CV_CAP_PROP_FRAME_COUNT);
    vector<vector<cv::Point> > detected_point_list(MAX_FRAMES, 
                                                   vector<cv::Point>(4));

    cout << "# Video file name: " << in_file_name << endl;
    cout << "# Total frames: " << MAX_FRAMES << endl;
    cout << "# frame number,pt1.x,pt1.y,pt2.x,pt2.y,pt3.x,pt3.y,pt4.x,pt4.y";
    cout << endl << "# Blob detection parameters set at frame number ";
    cout << start_from << endl;
    BlobParams blob_params = GetBlobParams(GetFrame(video_in_capture, 
                                                    start_from - 1));
    AutoTrackProgress auto_track_progress = AutoTrackProgress(blob_params,
                         MAX_FRAMES, start_from-1, blob_params.blobs_to_track);
    auto_track_progress = AutoTrack(video_in_capture, auto_track_progress, 
                                    detected_point_list);
    while (!auto_track_progress.analysis_complete)
    {
        cv::Mat frame = GetFrame(video_in_capture, 
                                 auto_track_progress.analyzed_till);
        cout << "# Blob detection parameters set at frame number ";
        cout << auto_track_progress.analyzed_till + 1 << endl;
        auto_track_progress.blob_params = GetBlobParams(frame);
        auto_track_progress.previous_frame_key_point_list = 
            auto_track_progress.blob_params.blobs_to_track;
        auto_track_progress = AutoTrack(video_in_capture, auto_track_progress, 
                                        detected_point_list);
    }
    cout << "# Digitization done." << endl;
    video_in_capture.release();
    return 0;
}
