#include "BlobParam.hpp"

const int MAX_circularity = 100,
          MAX_threshold = 255,
          MAX_area = 1000,
          MAX_convexity = 100;

int min_threshold = 100,
    max_threshold = 170,
    min_area = 100,
    max_area = 300,
    min_circularity = 30,
    min_convexity = 40;

bool need_to_update = false, recalculate = false;

cv::SimpleBlobDetector::Params params;
cv::Mat image, display_image;
vector<cv::KeyPoint> blobs_to_track(4);
vector<cv::KeyPoint> detected_blobs;
cv::Ptr<cv::SimpleBlobDetector> detector;

vector<cv::KeyPoint> DetectBlobs(cv::Mat frame_in, 
                                 cv::Ptr<cv::SimpleBlobDetector> detector)
{
    vector<cv::KeyPoint> key_point_list;
    detector->detect(frame_in, key_point_list);
    return key_point_list;
}

cv::SimpleBlobDetector::Params BlobDetectorParams()
{
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
    return params;
}

const char* WINDOW_PARAM = "Blob parameters & blobs-to-track selector";
const char* WINDOW_IMAGE = "Detected blobs";

int GetClickedKeyPoint(int x, int y)
{
    cv::KeyPoint current_key_point;
    for (int i=0; i < detected_blobs.size(); i++)
    {
        current_key_point = detected_blobs[i];
        if (sqrt(pow((x-current_key_point.pt.x), 2) + 
                 pow((y-current_key_point.pt.y), 2)) <= current_key_point.size)
        {
            return i;
        }
    }
    return -1;
}

bool IsKeyPointSame(cv::KeyPoint key_point_1, cv::KeyPoint key_point_2)
{
    return (key_point_1.pt.x == key_point_2.pt.x && 
            key_point_1.pt.y == key_point_2.pt.y);
}

void SortBlobsToTrack()
{
   return; 
}

static void UpdateDisplay()
{
    cv::drawKeypoints(image, detected_blobs, display_image, 
                      cv::Scalar(0, 0, 255), 
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::drawKeypoints(display_image, blobs_to_track, display_image, 
                      cv::Scalar(0, 255, 0), 
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::resize(display_image, display_image, cv::Size(), 0.7, 0.7);
    cv::imshow(WINDOW_PARAM, display_image);
}

static void MouseClickHandler(int event, int x, int y, int, void*)
{
    if (event != cv::EVENT_LBUTTONUP){return;}
    int clicked_key_point_number = GetClickedKeyPoint(x/0.7, y/0.7);
    if (clicked_key_point_number == -1) {return;}
    cv::KeyPoint clicked_key_point = detected_blobs[clicked_key_point_number];
    for (int i=0; i<4; i++)
    {
        if (IsKeyPointSame(clicked_key_point, blobs_to_track.at(i)))
        {
            blobs_to_track.at(i) = cv::KeyPoint(-1, -1, 0);
            SortBlobsToTrack();
            need_to_update = true;
            return;
        }
    }
    for (int i=0; i<4; i++)
    {
        if (IsKeyPointSame(blobs_to_track.at(i), cv::KeyPoint(-1, -1, 0)))
        {
            blobs_to_track.at(i) = clicked_key_point;
            SortBlobsToTrack();
            need_to_update = true;
            return;
        }
    }
}

static void param_change(int, void*)
{
    recalculate = true;
}

static void ReCalculate()
{
    params = BlobDetectorParams();
    detector = cv::SimpleBlobDetector::create(params);
    detected_blobs = DetectBlobs(image, detector);
    for (int i=0; i<4; i++)
    { 
        blobs_to_track[i] = cv::KeyPoint(-1, -1, 0);
    }
    need_to_update = true;
    if (detected_blobs.size() >= 4)
    {
        cv::setMouseCallback(WINDOW_PARAM, MouseClickHandler, 0);
    }
    else
    {
        cv::setMouseCallback(WINDOW_PARAM, NULL, NULL);
    }
}

static void SetupGUI()
{
    cv::namedWindow(WINDOW_PARAM, cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("minThreshold", WINDOW_PARAM, &min_threshold, 
                       MAX_threshold, param_change);
    cv::createTrackbar("maxThreshold", WINDOW_PARAM, &max_threshold, 
                       MAX_threshold, param_change);
    cv::createTrackbar("minArea", WINDOW_PARAM, &min_area, 
                       MAX_area, param_change);
    cv::createTrackbar("maxArea", WINDOW_PARAM, &max_area, 
                       MAX_area, param_change);
    cv::createTrackbar("minCircularity", WINDOW_PARAM, &min_circularity, 
                       MAX_circularity, param_change);
    cv::createTrackbar("minConvexity", WINDOW_PARAM, &min_convexity, 
                       MAX_convexity, param_change);
}


BlobParams GetBlobParams(cv::Mat input_image)
{
    image = input_image.clone();
    SetupGUI();
    param_change(0, 0);
    while ((char)cv::waitKey(100) != 'a')
    {
        if(need_to_update)
        {
            UpdateDisplay();
            need_to_update = false;
        }
        if(recalculate)
        {
            ReCalculate();
            recalculate = false;
        }
    }
    BlobParams blob_params;
    blob_params.params = params;
    blob_params.blobs_to_track = blobs_to_track;
    blob_params.detector = detector;
    cv::destroyAllWindows();
    cout << "# min threshold: " << blob_params.params.minThreshold << endl;
    cout << "# max threshold: " << blob_params.params.maxThreshold << endl;
    cout << "# min area: " << blob_params.params.minArea << endl;
    cout << "# max area: " << blob_params.params.maxArea << endl;
    cout << "# min circularity: " << blob_params.params.minCircularity << endl;
    cout << "# min convexity: " << blob_params.params.minConvexity << endl;
    return blob_params;
}
