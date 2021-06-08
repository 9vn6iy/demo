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
// 548 * 8
#define PACKET_SIZE 1024
#define UDP_HEADER_SIZE 64
#define SERVER_PORT "6201"
#define SERVER_IP "10.252.152.130"
#define SERVER_58_IP "10.178.21.11"

long getDataSizeBits(repeat) {
  return (PACKET_SIZE + UDP_HEADER_SIZE) * repeat;
}

void fill(char a[], int n) {
  for (int i = 0; i < n; i++) {
    a[i] = '1';
  }
}

int main(int argc, char const *argv[]) {
  clock_t start, end;
  double durationSec = 0.0, totalDurationSec = 0.0;
  double avgDurationSec = 0.0;
  char data[PACKET_SIZE];
  long dataSize, totalDataSize = 0L;
  // size_t dataLen = strlen(data);
  size_t dataLen = sizeof(data);
  fill(data, PACKET_SIZE);
  int clntSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (clntSock < 0) {
    DieWithSystemMessage("socket() failed");
  }
  struct sockaddr_in servAddr;
  socklen_t servAddrLen = sizeof(servAddr);
  memset(&servAddr, 0, servAddrLen);
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET, SERVER_IP, &servAddr.sin_addr);
  servAddr.sin_port = htons(atoi(SERVER_PORT));
  for (int i = 0; i < 50; i++) {
    start = clock();
    for (int j = 0; j < REPEAT_TIME; j++) {
      ssize_t numBytes = sendto(clntSock, data, dataLen, 0, 
          (struct sockaddr *)&servAddr, servAddrLen);
      if (numBytes < 0) {
        DieWithSystemMessage("sendto() failed");
      } else if (numBytes != dataLen) {
        DieWithUserMessage("sendto() error", "sent unexpected number of bytes");
      }
    }
    dataSize = getDataSizeBits(REPEAT_TIME);
    end = clock();
    durationSec = ((end - start) / CLOCKS_PER_SEC);
    totalDurationSec += durationSec;
    totalDataSize += dataSize;
    printf("duration = %f sec, ", durationSec);
    // printf("throught = %f bit/sec\n", dataSize / durationSec);
    printf("bitrate = %f mbps\n", (dataSize / durationSec) / 1000000);
  }
  close(clntSock);
  avgDurationSec = totalDurationSec / 50.0;
  printf("total duration = %f sec\n", avgDurationSec);
  // printf("total throught = %f bit/sec\n", totalDataSize / totalDurationSec);
  printf("total throught = %f mbps\n", (totalDataSize / totalDurationSec) / 1000000);
}


