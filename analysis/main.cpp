#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#define GAUSSIAN_BLUR_SIZE 11,11

using namespace std;

const int MAX_circularity = 100,
          MAX_threshold = 255,
          MAX_area = 1000,
          MAX_convexity = 100,
          MAX_inertia = 100;

int min_threshold = 139,
    max_threshold = 165,
    min_area = 170,
    max_area = 270,
    min_circularity = 70,
    min_convexity = 85,
    min_inertia = 30,
    frame_position = 0,
    MAX_frames,
    previous_frame_number = 0;

bool need_to_update = true,
     auto_proceed = false;
int keyboard = 0;
const char* WINDOW_MAIN = "Main window";
cv::Mat frame_in, frame_gray, frame_temp, frame_display;

vector<cv::KeyPoint> detect_blobs(cv::Mat frame_in)
{
    vector<cv::KeyPoint> key_point_list;
    cv::SimpleBlobDetector::Params params;
    params.thresholdStep = 1;
    params.minRepeatability = 10;
    params.blobColor = 255;
    params.minThreshold = min_threshold;
    params.maxThreshold = max_threshold;
    params.filterByArea = true;
    params.minArea = min_area;
    params.maxArea = max_area;
    params.filterByCircularity = true;
    params.minCircularity = float(min_circularity) / 100;
    params.filterByConvexity = true;
    params.minConvexity = float(min_convexity) / 100;
    params.filterByInertia = true;
    params.minInertiaRatio = float(min_inertia) / 100;
    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);
    detector->detect(frame_in, key_point_list);
    need_to_update = false;
    return key_point_list;
}

vector<cv::KeyPoint> analyze_frame(cv::VideoCapture video, int frame_number)
{
    if (previous_frame_number != frame_number - 1)
    {
        video.set(CV_CAP_PROP_POS_FRAMES, frame_number-1);
    }
    if(!video.read(frame_in))
    {
        cerr << "Unable to read next frame." << endl;
        cerr << "Exiting..." << endl;
        exit(EXIT_FAILURE);
    }
    cv::cvtColor(frame_in, frame_gray, cv::COLOR_RGB2GRAY);
    cv::GaussianBlur(frame_gray, frame_gray, cv::Size(GAUSSIAN_BLUR_SIZE), 0, 0, cv::BORDER_DEFAULT);
    previous_frame_number = frame_number;
    return detect_blobs(frame_gray);
}

static void param_change(int, void*)
{
    frame_position = cv::getTrackbarPos("frameNum", WINDOW_MAIN);
    min_threshold = cv::getTrackbarPos("minimumThreshold", WINDOW_MAIN);
    max_threshold = cv::getTrackbarPos("maxThreshold", WINDOW_MAIN);
    min_area = cv::getTrackbarPos("minArea", WINDOW_MAIN);
    max_area = cv::getTrackbarPos("maxArea", WINDOW_MAIN);
    min_circularity = cv::getTrackbarPos("minCircularity", WINDOW_MAIN);
    min_convexity = cv::getTrackbarPos("minConvexity", WINDOW_MAIN);
    min_inertia = cv::getTrackbarPos("minInertia", WINDOW_MAIN);
    need_to_update = true;
}

static void on_mouse(int event, int x, int y, int, void*)
{
    return;
}

static void setup_gui()
{
    cv::namedWindow(WINDOW_MAIN, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(WINDOW_MAIN, on_mouse, 0 );
    cv::createTrackbar("frameNum", WINDOW_MAIN, &frame_position, MAX_frames-1, param_change);
    cv::createTrackbar("minThreshold", WINDOW_MAIN, &min_threshold, MAX_threshold, param_change);
    cv::createTrackbar("maxThreshold", WINDOW_MAIN, &max_threshold, MAX_threshold, param_change);
    cv::createTrackbar("minArea", WINDOW_MAIN, &min_area, MAX_area, param_change);
    cv::createTrackbar("maxArea", WINDOW_MAIN, &max_area, MAX_area, param_change);
    cv::createTrackbar("minCircularity", WINDOW_MAIN, &min_circularity, MAX_circularity, param_change);
    cv::createTrackbar("minConvexity", WINDOW_MAIN, &min_convexity, MAX_convexity, param_change);
    cv::createTrackbar("minInertia", WINDOW_MAIN, &min_inertia, MAX_inertia, param_change);
}

static void dump_data(const char* data_out_file_name, string video_file_name, int MAX_frames, vector<vector<cv::Point> > detected_point_list)
{
    ofstream data_out;
    data_out.open(data_out_file_name, ios::out);
    data_out << "# video file: " << video_file_name ;
    data_out << "\n# min threshold: " << min_threshold;
    data_out << "\n# max threshold: " << max_threshold;
    data_out << "\n# min area: " << min_area;
    data_out << "\n# max area: " << max_area;
    data_out << "\n# min circularity: " << min_circularity;
    data_out << "\n# min convexity: " << min_convexity;
    data_out << "\n# min inertia: " << min_inertia;
    data_out << "\n# frame number, pt1.x, pt1.y, pt2.x, pt2.y, pt3.x, pt3.y, pt4.x, pt4.y";
    for (int i = 0; i < MAX_frames; i++)
    {
        data_out << "\n" << i << ",";
        for (int j = 0; j < 4; j++)
        {
            data_out << detected_point_list[i][j].x << "," << detected_point_list[i][j].y;
            if (j < 3)
            {
                data_out << ",";
            }
        }
    }
    data_out.close();
}

static void set_frame(int frame_number)
{
    cv::setTrackbarPos("frameNum",WINDOW_MAIN,frame_number);
}

static void handle_keyboard()
{
    keyboard = cv::waitKey(30);
    if (keyboard!=-1)
    {
    	cout << keyboard << endl;
    }
    if ( (char)keyboard == 'a')
    {
        auto_proceed = !auto_proceed;
        cout << "Auto proceed: " << auto_proceed << endl;
        need_to_update = true;
    }
    else if (keyboard == 32 || auto_proceed)
    {
        if (frame_position < MAX_frames-1)
        {
            set_frame(frame_position + 1);
        }
        else
        {
            auto_proceed = !auto_proceed;
            cv::displayOverlay(WINDOW_MAIN, "Reached last frame",5000);
        }
    }
    else if (keyboard == 65360)//home
    {
        set_frame(0);
    }
    else if (keyboard == 65367)//end
    {
        set_frame(MAX_frames - 1);
    }
    else if (keyboard == 65365)//pageup
    {
        set_frame(((frame_position - 100) < 0) ? 0 : (frame_position - 100));
    }
    else if (keyboard == 65366)//pagedown
    {
        set_frame(((frame_position + 100) >= MAX_frames ) ? MAX_frames : (frame_position + 100));
    }
}

int main(int argc, char *argv[])
{
    cout << boolalpha; //prints true or false instead of 1 or 0
    const string video_in_file = argv[1]; //first argument is input video file
    const char* data_out_file = argv[2]; //second argument is output data file
    cv::VideoCapture video_in_capture(video_in_file);
    if (!video_in_capture.isOpened()) {
        cerr  << "Could not open video " << video_in_file << endl;
        exit(EXIT_FAILURE);
    }
    MAX_frames = video_in_capture.get(CV_CAP_PROP_FRAME_COUNT);
    vector<cv::KeyPoint> detected_key_point_list;
    vector<vector<cv::Point> > detected_point_list(MAX_frames, vector<cv::Point>(4));
    setup_gui();
    while( (char)keyboard != 'q' ) {
        if(need_to_update) {
            detected_key_point_list = analyze_frame(video_in_capture, frame_position);
            cv::drawKeypoints( frame_gray, detected_key_point_list, frame_temp, cv::Scalar(0,0,255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
            cv::resize(frame_temp, frame_display, cv::Size(880,550));
            imshow("Main window", frame_display);
            if (detected_key_point_list.size() != 4) {
                auto_proceed = false;
                stringstream message;
                message << "Found " << detected_key_point_list.size() << " points instead of 4";
                cv::displayOverlay(WINDOW_MAIN, message.str(), 5000);
            }
            else {
                for (int i = 0; i < detected_key_point_list.size(); i++) {
                    detected_point_list[frame_position][i]=detected_key_point_list[i].pt;
                }
            }
            need_to_update = false;
        }
        handle_keyboard();
    }
    video_in_capture.release();
    dump_data(data_out_file, video_in_file, MAX_frames, detected_point_list);
}
