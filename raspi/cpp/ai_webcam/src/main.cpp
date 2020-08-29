#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <signal.h>
#include <thread>
#include <mutex>
#include "discovery.h"
#include "broadcaster.h"
#include "detector.h"

using namespace std;
using namespace cv;

#define CAPTURE_INTERVAL 20000  // 20msec

#define BROADCAST_PORT 18082

#define ANGLE_OF_VIEW 46  // camera's vertical FOV in degrees

char LOOPBACK_ADDR[] = "127.0.0.1";

bool proximitySensing = false;
bool equalize = false;
bool verbose = false;
bool show = false;
bool record = false;

// Advertisment thread
thread ad;

// Capture threads
thread dr;
thread cp;

// Broadcast thread 
char *broadcasterAddr = LOOPBACK_ADDR;
char *deviceId = NULL;
thread br;

// Object detection accuracy threshold
float accThres = 0.6;
int jpegQuality = 50;

// Camera's vertical FOV
int angleOfView = ANGLE_OF_VIEW;

// Command line options
const char optString[] = "d:b:t:pq:esra:v";

// Display command usage
void displayUsage(void) {
  cout << "Usage: server [OPTION...]" << endl;
  cout << "   -d deviceId                device ID" << endl;
  cout << "   -b broadcaster             broadcaster's IP address" << endl;
  cout << "   -t threshold               accuracy threshold (%)" << endl;
  cout << "   -p                         proximity sensing" << endl;
  cout << "   -q quality                 JPEG quality (%)" << endl;
  cout << "   -e                         histgram equalization" << endl;
  cout << "   -s                         show image" << endl;
  cout << "   -r                         record video" << endl;
  cout << "   -a angleOfView             angle of view" << endl;
  cout << "   -v                         verbose output" << endl;
}

// Command argument parser
void argparse(int argc, char *argv[]) {

  int opt;
  opterr = 0;  // disable getopt() error message output

  while ( (opt = getopt(argc, argv, optString)) != -1 ) {
    switch(opt) {
      case 'b':
        broadcasterAddr = optarg;
        break;
      case 'd':
        deviceId = optarg;
        break;
      case 't':
        accThres = atoi(optarg) / 100.0;
        break;
      case 'p':
        proximitySensing = true;
        break;
      case 'q':
        jpegQuality = atoi(optarg);
        break;
      case 'e':
        equalize = true;
        break;
      case 's':
        show = true;
        break;
      case 'r':
        record = true;
        break;
      case 'a':
        angleOfView = atoi(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      default:
        displayUsage();
        exit(-1);
        break;
    }
  }
}

// Signal handler for keyboard interruput
void sigHandler(int s) {
  stopBroadcast(); 
  stopAdvertise();
  stopProcess();
  br.join();
  ad.join();
  cp.join();
  cout << "Process terminated!" << endl;
  exit(0);
}

// Main procedure
int main(int argc, char *argv[]) {

  // Signal handler settings
  struct sigaction sigAction;
  sigAction.sa_handler = sigHandler;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = 0;
  sigaction(SIGINT, &sigAction, NULL);

  // Ignore broken socket error event
  signal(SIGPIPE, SIG_IGN);

  // Parse command line arguments
  argparse(argc, argv);
  if (deviceId == NULL) {
    cerr << "-d option (Device ID) is mandatory!" << endl;
    exit(-1);
  }

  cout << "[Camera unit]" << endl;
  cout << "DeviceId: " << deviceId << endl;
  cout << "Broadcaster IP address: " << broadcasterAddr << endl;
  cout << endl;

  // Start advertisment in a thread
  ad = thread(advertise, deviceId);

  // Start capture in a thread
  dr = thread(drain, jpegQuality, proximitySensing, equalize, show, record, angleOfView, verbose);
  cp = thread(process, accThres, verbose);

  // Start broadcast in a thread
  br = thread(broadcast, broadcasterAddr, deviceId);

  while(1) {

  }

}

