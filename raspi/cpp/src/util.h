#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void equalizeHistForBGR(Mat &img);

float calcEstimatedDistance(int angleOfView, int personHeight, int ratio);

float calcEstimatedDistance(int angleOfView, int personHeight, int screenHeight, int areaHeight);

