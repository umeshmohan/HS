#include "BlobParam.hpp"
#include "AutoTrack.hpp"
#define USAGE "Usage:\nHSD <video_file> [start_from_frame_number(default=0)] [MAX_DISTANCE(default=0.5r)] [DISPLAY_SCALE(default=0.7)]\n\n"
using namespace std;
float MAX_DISTANCE, DISPLAY_SCALE;
int BLOB_COLOR;

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

cv::VideoCapture OpenVideo(string video_file)
{
    cv::VideoCapture video_in_capture(video_file);
    if (!video_in_capture.isOpened()) {
        cerr  << "Could not open video " << video_file << endl;
        exit(EXIT_FAILURE);
    }
    return video_in_capture;
}

int main(int argc, char *argv[])
{
    cmdline::parser a;
    a.add<string>("file", 'f', "Input file for digitization", true, "");
    a.add<int>("start-frame", 's', "Start digitizing from frame", false, 0);
    a.add<float>("max-distance", 'd', 
        "Maximum distance from previous frame for a blob to be considered \
same (multiple of radius of blob)", 
        false, 0.5);
    a.add<float>("display-scale", 'x', "Scale displayed frame", false, 0.6);
    a.add<int>("blob-color", 'c', "Blob color for detection", false, 255, 
               cmdline::oneof<int>(0, 255));
    a.parse_check(argc, argv);

    const string in_file_name = a.get<string>("file");
    int start_from = a.get<int>("start-frame");
    
    MAX_DISTANCE = a.get<float>("max-distance");
    DISPLAY_SCALE = a.get<float>("display-scale");
    BLOB_COLOR = a.get<int>("blob-color");
    
    std::ios_base::openmode mode1 = ios_base::app; 
    ofstream out((in_file_name.substr(0,in_file_name.length()-3) 
                 + "csv").c_str(), mode1);
    std::cout.rdbuf(out.rdbuf());
    
    cv::VideoCapture video_in_capture = OpenVideo(in_file_name);
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
    auto_track_progress = AutoTrack(video_in_capture, auto_track_progress);
    while (!auto_track_progress.analysis_complete)
    {
        cv::Mat frame = GetFrame(video_in_capture, 
                                 auto_track_progress.analyzed_till);
        cout << "# Blob detection parameters set at frame number ";
        cout << auto_track_progress.analyzed_till + 1 << endl;
        auto_track_progress.blob_params = GetBlobParams(frame);
        auto_track_progress.previous_frame_key_point_list = 
            auto_track_progress.blob_params.blobs_to_track;
        auto_track_progress = AutoTrack(video_in_capture, auto_track_progress);
    }
    cout << "# Digitization done." << endl;
    video_in_capture.release();
    cv::destroyAllWindows();
    return 0;
}
