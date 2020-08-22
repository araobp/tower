#include <iostream>
#include <string>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <mutex>
#include <time.h>

#include "util.h"

// Headers for tensorflow-lite
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/tools/gen_op_registration.h"

// SSD MobileNet v2 pre-trained model
#define MODEL_FILE "detect.tflite"
#define LABEL_FILE "labelmap.txt"
#define TFLITE_INPUT_WIDTH 300
#define TFLITE_INPUT_HEIGHT 300

// The number of threads in a tensorflow-lite's thread pool
#define NUM_THREADS 3

// JPEG resolution for capturing image from UVC device
#define JPEG_WIDTH 1280  // in pixels
#define JPEG_HEIGHT 720  // in pixels
#define FPS 30  // frames per second

// Alert frame thickness
#define ALERT_THICKNESS 32 
const int alertThicknessHalf = ALERT_THICKNESS/2;

// Recording duration
//#define RECORDING_DURATION 10000  // 10sec
#define RECORDING_DURATION 180000  // 180sec

// Distance estimation
#define PERSON_HEIGHT_STD 170  // cm
#define PERSON_WIDTH_STD 70  // cm
#define ANGLE_OF_VIEW 67 // degrees
#define PERSON_HEIGHT_RATIO 25 // %
const int personMinHeightInPixels = PERSON_HEIGHT_RATIO * JPEG_HEIGHT / 100.0;  
const int personMinWidthInPixels = personMinHeightInPixels * PERSON_WIDTH_STD / PERSON_HEIGHT_STD; 
const float Y1 = (JPEG_HEIGHT - personMinHeightInPixels) / 2.0;
const float Y2 = Y1 + personMinHeightInPixels;
const float personWidthHalf = personMinHeightInPixels * PERSON_WIDTH_STD / PERSON_HEIGHT_STD / 2.0;
const float X1 = JPEG_WIDTH / 2.0 - personWidthHalf;
const float X2 = JPEG_WIDTH / 2.0 + personWidthHalf;
const auto X1Y1 = cv::Point(X1, Y1);
const auto X2Y2 = cv::Point(X2, Y2);
const auto alertX1Y1 = cv::Point(alertThicknessHalf, alertThicknessHalf);
const auto alertX2Y2 = cv::Point(JPEG_WIDTH-alertThicknessHalf, JPEG_HEIGHT-alertThicknessHalf);

using namespace std;
using namespace cv;

// Video caputure from USB Video Camera (UVC) device
VideoCapture cap(0);

// Writer
VideoWriter writer;

bool captureRunning = true;

std::vector<unsigned char> jpegImage;
std::vector<int> param(2);

mutex copyLock;
mutex captureLock;
mutex resultsLock;

const int colors[10][3] = {
  {255, 255, 0}, {0, 255, 255}, {128, 256, 128}, {64, 192, 255}, {128, 128, 255},
  {255, 255, 0}, {0, 255, 255}, {128, 256, 128}, {64, 192, 255}, {128, 128, 255}};

vector<unsigned char> img;

auto white = cv::Scalar(255, 255, 255);
auto black = cv::Scalar(0, 0, 0);

bool captureReady = false;

struct result {
  int top;
  int bottom;
  int left;
  int right;
  float score;
  std::string className;
};

vector<result> results;

Mat mat(Size(JPEG_WIDTH, JPEG_HEIGHT), CV_8UC3);
Mat mat2(Size(JPEG_WIDTH, JPEG_HEIGHT), CV_8UC3);

bool alert = false;

int sno = 0;
chrono::steady_clock::time_point prevTime;

void openVideoFile(float fps) {
  char filename[32];
  time_t now = time(0);
  struct tm* it = localtime(&now);
  sprintf(filename, "/home/pi/video/%04d%02d%02d_%02d%02d%02d-%d.avi",
          it->tm_year + 1900, it->tm_mon + 1, it->tm_mday,
          it->tm_hour, it->tm_min, it->tm_sec, sno++);
  writer.open(filename, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(JPEG_WIDTH, JPEG_HEIGHT), true);
  prevTime = chrono::steady_clock::now();
  cout << "Video file: " << filename << endl; 
}

void getJpegImage(char *jpegArray, size_t *size, bool *personDetected) {
  *personDetected = alert;
  *size = (size_t)jpegImage.size();
  std::memcpy(jpegArray, jpegImage.data(), *size);
}

void stopProcess() {
  captureRunning = false;
  cap.release();
}

void drain(int jpegQuality, bool proximitySensing, bool equalize, bool show, bool record, bool verbose) {
  float distanceRange = calcEstimatedDistance(ANGLE_OF_VIEW, PERSON_HEIGHT_STD, PERSON_HEIGHT_RATIO);
  cout << "Distance range: " << distanceRange << "(m)" << endl;
  
  /** Video caputure initialization */
  //cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));  // Motion JPEG
  //cap.set(CAP_PROP_CONVERT_RGB, false);  // raw JPEG (not BGR)
  cap.set(CV_CAP_PROP_FRAME_WIDTH, JPEG_WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, JPEG_HEIGHT);
  cap.set(CV_CAP_PROP_FPS, FPS);

  if (!cap.isOpened()) {
    cerr << "USB cam is not connected!" << endl;
    exit(-1);
  }

  cout << "USB cam opened." << endl;

  param[0] = cv::IMWRITE_JPEG_QUALITY;
  param[1] = jpegQuality;  //default(95) 0-100
  
  int frameCnt = 0;
  float fps = 0.0;

  while (captureRunning) {
    auto startCap = chrono::steady_clock::now();
    
    if (!cap.read(mat)) {
      cerr << "Capture failed!" << endl;
    } else {

      // Apply histgram equalization to the image
      if (equalize) equalizeHistForBGR(mat);

      if (record) {
        auto currentTime = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - prevTime).count(); 
        if (frameCnt < 0) {
          writer << mat;
          if (elapsedTime > RECORDING_DURATION) {
            writer.release();
            openVideoFile(fps);
          }
        } else {
          if (frameCnt == 0) {
            prevTime = chrono::steady_clock::now();
            cout << "FPS measurement start..." << endl;
            frameCnt = 1;
          } else if (++frameCnt >= 100) {
            fps = 100.0 * 1000.0 / elapsedTime;
            cout << "FPS: " << fps << endl;
            frameCnt = -1; 
            openVideoFile(fps);
          }
        }
      }

      //<<<
      captureLock.lock();
      mat2 = mat.clone();
      captureLock.unlock();
      //>>>
      auto startDraw = chrono::steady_clock::now();

      int i = 0;
      bool personDetected = false;
      float distance = 0.0;

      //<<<
      resultsLock.lock();
      for (auto r: results) {
        int R, G, B, thickness;
        int areaWidth = r.right - r.left;
        int areaHeight = r.bottom - r.top;
        if ( !proximitySensing || ( proximitySensing && (areaHeight >= personMinHeightInPixels || areaWidth >= personMinWidthInPixels) ) ) {
          string label = r.className + " " + std::to_string(r.score).substr(0, 4) + "%";
          if (r.className == "person" || r.className == "worker" || r.className == "guard") {
            B = 0;
            G = 0;
            R = 255;
            thickness = 3;
            personDetected = true;
            distance = calcEstimatedDistance(ANGLE_OF_VIEW, PERSON_HEIGHT_STD,
                JPEG_HEIGHT, areaHeight);
            label += " " + std::to_string(distance).substr(0,4) + "m";
          } else {
            B = colors[i][0];
            G = colors[i][1];
            R = colors[i][2];
            thickness = 1;
            personDetected = false;
          }
          auto color = cv::Scalar(B, G, R);
          cv::rectangle(mat, cv::Point(r.left, r.top), cv::Point(r.right, r.bottom), color, thickness);
          cv::rectangle(mat, cv::Point(r.left, r.top+20), cv::Point(r.left+170, r.top), color, CV_FILLED);
          cv::putText(mat, label, cv::Point(r.left, r.top+15), cv::FONT_HERSHEY_SIMPLEX, 0.5, black);
          if (personDetected) cv::rectangle(mat, alertX1Y1, alertX2Y2, color, ALERT_THICKNESS);
          i++;
        }
      }
      resultsLock.unlock();
      //>>>

      // Minimum person height
      cv::rectangle(mat, X1Y1, X2Y2, white, 1);

      if (show) {
        imshow("camera", mat);
        waitKey(1);
      }

      auto startEncode = chrono::steady_clock::now();
      copyLock.lock();
      alert = personDetected; 
      cv::imencode(".jpg", mat, jpegImage, param);
      copyLock.unlock();
      auto end = chrono::steady_clock::now();

      if (verbose) {
        cout << "---" << endl;
        cout << "Capture: " << chrono::duration_cast<chrono::milliseconds>(startDraw-startCap).count() << " msec" << endl;
        cout << "Draw: " << chrono::duration_cast<chrono::milliseconds>(startEncode-startDraw).count() << " msec" << endl;
        cout << "Encode: " << chrono::duration_cast<chrono::milliseconds>(end-startEncode).count() << " msec" << endl;
        cout << "Drain total: " << chrono::duration_cast<chrono::milliseconds>(end-startCap).count() << " msec" << endl;
      }
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int process(float accThres, bool verbose) {

  Mat resized(Size(TFLITE_INPUT_WIDTH, TFLITE_INPUT_HEIGHT), CV_8UC3);
  Mat input(Size(TFLITE_INPUT_WIDTH, TFLITE_INPUT_HEIGHT), CV_8UC3);
  
  /** TFLite inference preparation */

  // Load model and label map
  std::unique_ptr<tflite::FlatBufferModel> model;
  std::ifstream ifs;
  //model = tflite::FlatBufferModel::BuildFromFile(MODEL_FILE);
  model = tflite::FlatBufferModel::BuildFromFile(MODEL_FILE);
  ifs = ifstream(LABEL_FILE);
  if (!model) {
    cerr << "Unable to load TFlite model!" << endl;
    return -1;
  }
  if (!ifs) {
    cerr << "Unable to load class lable file!" << endl;
    return -1;
  }

  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  tflite::InterpreterBuilder(*model, resolver)(&interpreter);

  interpreter->AllocateTensors();
  interpreter->SetNumThreads(NUM_THREADS);

  vector<string> classNames;
  string line;
  while (getline(ifs, line)) {
    //cout << line << endl;
    classNames.push_back(line);
  }

  do {
    auto startResize = chrono::steady_clock::now();
    //<<<
    captureLock.lock();
    resize(mat2, resized, Size(), (float)TFLITE_INPUT_WIDTH/JPEG_WIDTH, (float)TFLITE_INPUT_HEIGHT/JPEG_HEIGHT);
    captureLock.unlock();
    //>>>
    auto startInput = chrono::steady_clock::now();
    cvtColor(resized, input, COLOR_BGR2RGB);

    uint8_t *input_tensor = interpreter->typed_tensor<uint8_t>(interpreter->inputs()[0]);
    int idx = 0;
    for (int row = 0; row < TFLITE_INPUT_HEIGHT; row++) {
      for (int col = 0; col < TFLITE_INPUT_WIDTH; col++) {
        for (int channel = 0; channel < 3; channel++) {
          input_tensor[idx] = input.data[idx]; 
          idx++;
        }
      }
    }

    auto startInference = chrono::steady_clock::now();
    // Execute inference
    interpreter->Invoke();

    // Get result
    float *result1 = interpreter->typed_output_tensor<float>(0); //Locations (Top, Left, Bottom, Right)
    float *result2 = interpreter->typed_output_tensor<float>(1); //Classes (0=Person)
    float *result3 = interpreter->typed_output_tensor<float>(2); //Scores
    float *result4 = interpreter->typed_output_tensor<float>(3); //Number of detections

    // Draw result
    vector<result> rr;
    for (int i = result4[0] - 1; i >= 0; i--) {
      int top    = result1[4*i + 0] * TFLITE_INPUT_HEIGHT;
      int left   = result1[4*i + 1] * TFLITE_INPUT_WIDTH;
      int bottom = result1[4*i + 2] * TFLITE_INPUT_HEIGHT;
      int right  = result1[4*i + 3] * TFLITE_INPUT_WIDTH;
      //#define SWAP(X,Y) {(X)+=(Y); (Y)=(X)-(Y); (X)-=(Y);}
      //if (left > right) SWAP(left, right);
      //if (top > bottom) SWAP(top, bottom);
      std::string className = classNames[result2[i]+1];
      float score = result3[i];
      if (score < accThres) continue;
      //std::cout << "Location=(" << left << "," << top << ")-(" << right << "," << bottom << "), ";
      //std::cout << "Class=" << classNames[result2[i]+1] << ", ";
      //std::cout << "Score=" << score << ", ";
      //std::cout << std::endl;
      left = (int)((float)left * JPEG_WIDTH / TFLITE_INPUT_WIDTH);
      right = (int)((float)right * JPEG_WIDTH / TFLITE_INPUT_WIDTH);
      top = (int)((float)top * JPEG_HEIGHT / TFLITE_INPUT_HEIGHT);
      bottom = (int)((float)bottom * JPEG_HEIGHT / TFLITE_INPUT_HEIGHT);
      result r;
      r.top = top;
      r.bottom = bottom;
      r.left = left;
      r.right = right;
      r.score = score;
      r.className = className;
      rr.push_back(r);
    }

    auto startRR = chrono::steady_clock::now();
    resultsLock.lock();
    results = rr;
    resultsLock.unlock();

    //cout << mat.cols << endl;
    //cout << mat.rows << endl;
    //cout << "---" << endl;

    auto end = chrono::steady_clock::now();

    if (verbose) {
      cout << "---"  << endl;
      cout << "Resize: " << chrono::duration_cast<chrono::milliseconds>(startInput-startResize).count() << " msec" << endl;
      cout << "Input: " << chrono::duration_cast<chrono::milliseconds>(startInference-startInput).count() << " msec" << endl;
      cout << "Inference: " << chrono::duration_cast<chrono::milliseconds>(startRR-startInference).count() << " msec" << endl;
      cout << "Results: " << chrono::duration_cast<chrono::milliseconds>(end-startRR).count() << " msec" << endl;
      cout << "Total: " << chrono::duration_cast<chrono::milliseconds>(end-startInput).count() << " msec" << endl;
    }

  } while (captureRunning);

  return 0;
}
