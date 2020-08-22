#include <math.h>
#include "util.h"

#define PI 3.14159265

/**
 * Reference:
 * https://www.opencv-srf.com/2018/02/histogram-equalization.html 
 */
void equalizeHistForBGR(Mat &img) {
  Mat equalized;
  vector<Mat> channels;

  cvtColor(img, equalized, COLOR_BGR2YCrCb);  // YCrCb 
  split(equalized, channels);
  equalizeHist(channels[0], channels[0]);  // equalization on Y channel
  merge(channels, equalized);
  cvtColor(equalized, img, COLOR_YCrCb2BGR);
}

float calcEstimatedDistance(int angleOfView, int personHeight, int ratio) {
  float heightMeter = personHeight / 100.0;
  float ratioFloat = ratio / 100.0;
  float thetaRadHalf = ratioFloat * angleOfView * PI / 180.0 / 2.0;
  float distance = heightMeter / 2.0 / tan(thetaRadHalf);
  return round((distance * 10.0))/10.0;
}

float calcEstimatedDistance(int angleOfView, int personHeight, int screenHeight, int areaHeight) {
  float heightMeter = personHeight / 100.0;
  float ratioFloat = (float)areaHeight / screenHeight;
  float thetaRadHalf = ratioFloat * angleOfView * PI / 180.0 / 2.0;
  float distance = heightMeter / 2.0 / tan(thetaRadHalf);
  return round((distance * 10.0))/10.0;
}

