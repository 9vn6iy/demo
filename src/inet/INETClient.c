#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>          
#include <arpa/inet.h>
#include <time.h>

#include "../../include/HandleError.h"
#include "../../include/constant.h"

#define REPEAT_TIME 100000
#define BUF_SIZE 64
#define PACKET_SIZE 64
#define FAC_SEC 10

double startTimes[REPEAT_TIME];
double finishTimes[REPEAT_TIME];
double durationSecTimes[REPEAT_TIME];
// double bitratespersec[fac_sec];
// double jitterpersec[fac_sec];

int main(int argc, char const *argv[]) {
  clock_t start, finish;
  int totalDataSize = 0;
  double totalTimeSec = 0;
  double duration = 0;
  char servIP[] = "10.252.152.130";
  char servPort[] = "6201";
  char data[PACKET_SIZE];

  size_t dataLen = strlen(data);
  int clntSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (clntSock < 0) {
    DieWithSystemMessage("socket() failed");
  }
  // char buffer[BUF_SIZE];
  struct sockaddr_in servAddr;
  socklen_t servAddrLen = sizeof(servAddr);
  bzero(&servAddr, servAddrLen);
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET, servIP, &servAddr.sin_addr);
  servAddr.sin_port = htons(atoi(servPort));
  double outsideStart = clock();
  double outsideEnd = 0;
  double totalDuration = 0;
  for (int j = 0; j < 50; j++) {
    start = clock();
    for (int i = 0; i < REPEAT_TIME; i++) {
      startTimes[i] = start;
      ssize_t numBytes = sendto(clntSock, data, dataLen, 0, 
          (struct sockaddr *)&servAddr, servAddrLen);
      if (numBytes < 0) {
        DieWithSystemMessage("sendto() failed");
      } else if (numBytes != dataLen) {
        DieWithUserMessage("sendto() error", "sent unexpected number of bytes");
      }
      /*
      struct sockaddr_storage fromAddr;
      socklen_t fromAddrLen = sizeof(fromAddr);
      char buffer[MAX_STRING_LENGTH + 1];
      numBytes = recvfrom(clntSock, buffer, MAX_STRING_LENGTH, 0,
          (struct sockaddr *)&fromAddr, &fromAddrLen);
      if (numBytes < 0) {
        DieWithUserMessage("recvfrom() error", "received unexpected number of bytes");
      }
      totalDataSize += numBytes * 8;
      finishTimes[i] = finish;
      double durationTimeMs = duration / CLOCKS_PER_SEC * 1000;
      double durationTimeSec = durationTimeMs / 1000;
      totalTimeSec += durationTimeSec;
      double bps = numBytes / durationTimeSec;
      double Mbps = bps / 8388608;
      durationSecTimes[i] = durationTimeSec;
      printf("spent time: %fms, ", durationTimeSec * 1000);
      printf("bitrate: %fMbits/sec\n", Mbps);
      */
    }
    finish = clock();
    duration = finish - start;
    totalDuration += duration;
    printf("duration = %f\n", duration);
  }
  outsideEnd = clock();
  close(clntSock);
  // printf("total = %dbit\n", totalDataSize);
  // printf("totalTime = %lf\n", totalTimeSec);
  double totalOutsideTime = (outsideEnd - outsideStart) / CLOCKS_PER_SEC * 1000;
  // printf("outsideTime = %lfms\n", totalOutsideTime);
  // printf("sp = %lf\n", totalDataSize / totalOutsideTime);
  // printf("end = %f, start = %f\n", outsideEnd, outsideStart);
  printf("total duration = %f\n", totalDuration / 50.0);
}
