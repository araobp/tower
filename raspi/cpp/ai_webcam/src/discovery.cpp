#include <iostream>
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "discovery.h"

#define INTERVAL 500  // Advertisement at 500msec interval
#define PORT 18084  // Advertisment UDP port

using namespace std;

bool advertiseRunning = false;

/**
 * Find IP broadcast address on wlan0 
 */
int getAdvertiseAddress(in_addr *bAddr) {
  
  struct ifaddrs *ifAddrStruct = NULL;
  struct ifaddrs *ifa = NULL;
  in_addr *pAddr = NULL;
  in_addr *pNetmask = NULL;
  in_addr tmpMask;
  in_addr tmpAddr;
  int code = -1;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) { // check if it is IP4
      if (strncmp(ifa->ifa_name, "wlan0", 5) == 0) {
        pAddr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        pNetmask=&((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr;
        tmpMask.s_addr = pNetmask->s_addr;
        tmpAddr.s_addr = pAddr->s_addr;
        tmpAddr.s_addr = tmpAddr.s_addr | ~tmpMask.s_addr;  // Broadcast address
        bAddr->s_addr = tmpAddr.s_addr;

        code = 0;
      }
    } 
  }
  if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
  return code;
}

void stopAdvertise() {
  advertiseRunning = false;
}

/**
 * Advertise an IP address of this host. 
 */
int advertise(char *deviceId) {

  char advertiseAddr[INET_ADDRSTRLEN];
  in_addr bAddr;

  int sockFd = 0;
  struct sockaddr_in advertise_addr; 
    
  while (1) {
    if (getAdvertiseAddress(&bAddr) == 0) {
      // Convert IPv4 broadcast address from binary to text 
      inet_ntop(AF_INET, &bAddr, advertiseAddr, INET_ADDRSTRLEN);
      cout <<  "[Advertisment]" << endl; 
      cout <<  "IP address: " << advertiseAddr << endl; 
      cout <<  "UDP port: " << PORT << endl; 
      cout << endl;
      break;
    } else {
      cout << "IPv4 broadast address not found" << endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  if ( ( sockFd = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 ) {
    cerr << "Socket creation error!" << endl;
    return -1;
  }

  // Set broadcast address and port
  advertise_addr.sin_family = AF_INET;
  advertise_addr.sin_port = htons(PORT);
  advertise_addr.sin_addr.s_addr = bAddr.s_addr;

  // Get broadcast permission
  int broadcastEnable=1;
  if ( (setsockopt(sockFd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) ) < 0) {
    cerr << "Unable to get broadcast permission!";
    return -1;
  }
  
  // Broadcast ai-webcam discovery message at a certain interval
  advertiseRunning = true;
  while (advertiseRunning) {
    if ((sendto(sockFd, deviceId, strlen(deviceId), MSG_CONFIRM,
        (const struct sockaddr *) &advertise_addr,
        sizeof(advertise_addr))) < 0) {
      cerr << "Send error!" << endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL));
    //cout << "..." << endl;
  }

  return 0;
}

