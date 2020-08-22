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
#include "broadcast.h"
#include "detector.h"

using namespace std;
using namespace cv;

#define CAPTURE_INTERVAL 20000  // 20msec

#define BROADCAST_PORT 18082

#define BUFSIZE 300000  // 300Kbytes
char jpegArray[BUFSIZE];

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
char *broadcastAddress = NULL;
char *deviceId = NULL;
thread br;

// Object detection accuracy threshold
float accThres = 0.6;
int jpegQuality = 50;

// Command line options
const char optString[] = "b:d:t:pq:esrv";

// Display command usage
void displayUsage(void) {
  cout << "Usage: server [OPTION...]" << endl;
  cout << "   -b broadcast               broadcast server's IP address" << endl;
  cout << "   -d deviceId                device ID" << endl;
  cout << "   -t threshold               accuracy threshold (%)" << endl;
  cout << "   -p                         proximity sensing" << endl;
  cout << "   -q quality                 JPEG quality (%)" << endl;
  cout << "   -e                         histgram equalization" << endl;
  cout << "   -s                         show image" << endl;
  cout << "   -r                         record video" << endl;
  cout << "   -v                         verbose output" << endl;
}

// Command argument parser
void argparse(int argc, char *argv[]) {

  int opt;
  opterr = 0;  // disable getopt() error message output

  while ( (opt = getopt(argc, argv, optString)) != -1 ) {
    switch(opt) {
      case 'b':
        broadcastAddress = optarg;
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

  // Start advertisment in a thread
  ad = thread(advertise);

  // Start capture in a thread
  dr = thread(drain, jpegQuality, proximitySensing, equalize, show, record, verbose);
  cp = thread(process, accThres, verbose);

  // Start broadcast in a thread
  br = thread(broadcast, broadcastAddress, deviceId);

  cout << "[Camera unit]" << endl;
  cout << "DeviceId: " << deviceId << endl;
  cout << "Broadcast server: " << broadcastAddress << endl;

  while(1) {

  }

}


