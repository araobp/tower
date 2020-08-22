/**
 * Reference: https://stackoverflow.com/questions/26070899/use-c-to-send-a-file-using-http-poSt
 */
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "broadcast.h"
#include "detector.h"

#define PORT 18082
#define BUFSIZE 300000  // 300Kbytes
#define RECV_BUFSIZE 1024  // 1024bytes
#define INTERVAL 100000 // 100msec
#define INTERVAL2 5000000 // 5000msec

using namespace std;

bool broadcastRunning = false;

void stopBroadcast() {
  broadcastRunning = false;
}

void broadcast(char *broadcastAddress, char *deviceId) {

  int sock = 0;
  int cnt = 0;
  struct sockaddr_in serv_addr;

  char jpegArray[BUFSIZE];
  size_t jpegSize;

  char recvBuf[RECV_BUFSIZE] = { 0 };
  bool alert;

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, broadcastAddress, &serv_addr.sin_addr)<=0)
	{
	  cerr << "Invalid address!" << endl;
	  return;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

  cout << "Broadcast started" << endl;
  broadcastRunning = true;
  while (broadcastRunning) {

    if (sock <= 0) {
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
        cerr << "Socket creation error!" << endl;
      }

      int err;
      do {
        err = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (err < 0) { 
          cerr << "Connection failed!" << endl;
        } else {
          cout << "Connected" << endl;
        }
        usleep(INTERVAL2);
      } while(err < 0);
	  }
    

    //<<<
    copyLock.lock();
    getJpegImage(jpegArray, &jpegSize, &alert); 
    copyLock.unlock();
    //>>>

    string sHeader = "POST /broadcast/" + string(deviceId) + " HTTP/1.1\r\n"
      + "Host: " + string(broadcastAddress) + ":" + to_string(PORT) + "\r\n"
      + "Content-Type: image/jpeg\r\n"
      + "Content-Length: " + to_string(jpegSize) + "\r\n"
      + "Connection: keep-alive\r\n"
      +"\r\n";

    //cout << sHeader << endl;

    char header[sHeader.size() + 1];
    strcpy(header, sHeader.c_str()); 
    //cout << header << endl;

    cnt = 0;
    int headerLen = strlen(header);
    //cout << headerLen << endl;
    do {
      cnt += send(sock, header, headerLen - cnt, 0);
      //cout << cnt << endl;
    } while (cnt > 0 && cnt < headerLen);
    //cout << "header" << endl;

    cnt = 0;
    do {
      cnt += send(sock, jpegArray, jpegSize - cnt, 0);
    } while (cnt > 0 && cnt < (int)jpegSize);
    // cout << "jpeg" << endl;

    cnt = read( sock, recvBuf, RECV_BUFSIZE );
    //printf("%s\n", recvBuf);
    
    if (cnt == 0) {  // Reconnect if network is disconnected
      cout << "Reconnect..." << endl;
      if (sock > 0) {
        close(sock);
        sock = 0;
      }
    }

    usleep(INTERVAL);
  }

  // Close socket if opened
  if (sock > 0) close(sock);

  return;
}

