#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "../../include/HandleError.h"
#include "../../include/constant.h"

#define REPEAT_TIME 1000
// #define BUF_SIZE 64
// 548 * 8
#define PACKET_SIZE 1024
#define UDP_HEADER_SIZE 64
#define SERVER_PORT "6201"
#define SERVER_IP "10.252.152.130"
#define SERVER_58_IP "10.178.21.11"
#define THREAD_COUNT 100

long getDataSizeBits(repeat) {
  return (PACKET_SIZE + UDP_HEADER_SIZE) * repeat;
}

void fill(char a[], int n) {
  for (int i = 0; i < n; i++) {
    a[i] = '1';
  }
}

struct ThreadData {
  int id;
  FILE *log;
  int repeatTime;
};

int initPort = 6201;

void *sendData(void *arg) {
  struct ThreadData *t = (struct ThreadData *)arg;
  time_t start, end;
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
  servAddr.sin_port = htons(initPort);
  for (int i = 0; i < 50; i++) {
    start = time(NULL);
    for (int j = 0; j < t->repeatTime; j++) {
      ssize_t numBytes = sendto(clntSock, data, dataLen, 0,
                                (struct sockaddr *)&servAddr, servAddrLen);
      if (numBytes < 0) {
        DieWithSystemMessage("sendto() failed");
      } else if (numBytes != dataLen) {
        DieWithUserMessage("sendto() error", "sent unexpected number of bytes");
      }
    }
    dataSize = getDataSizeBits(REPEAT_TIME);
    end = time(NULL);
    durationSec = (end - start);
    totalDurationSec += durationSec;
    totalDataSize += dataSize;
    // printf("duration = %f sec, ", durationSec);
    // printf("throught = %f bit/sec\n", dataSize / durationSec);
    // printf("bitrate = %f mbps\n", (dataSize / durationSec) / 1000000);
  }
  close(clntSock);
  avgDurationSec = totalDurationSec / 50.0;
  fprintf(t->log, "thread %2d total duration = %f sec, ", t->id,
          avgDurationSec);
  // printf("total throught = %f bit/sec\n", totalDataSize / totalDurationSec);
  fprintf(t->log, "total throught = %f mbps\n",
          (totalDataSize / totalDurationSec) / 1000000);
  return NULL;
}

// <repeat time, thread count>
int main(int argc, char const *argv[]) {
  int repeatTime, threadCount;
  if (argc < 3) {
    repeatTime = 1000;
    threadCount = 100;
  } else {
    repeatTime = atoi(argv[1]);
    threadCount = atoi(argv[2]);
  }
  pthread_t threads[threadCount];
  struct ThreadData td[threadCount];
  FILE *log = fopen("log.txt", "w");
  fprintf(log, "repeat count = %s, thread count = %s\n", argv[1], argv[2]);
  for (int i = 0; i < threadCount; i++) {
    td[i].id = i;
    td[i].log = log;
    td[i].repeatTime = repeatTime;
    fprintf(log, "create thread %d\n", i);
    int rc = pthread_create(&threads[i], NULL, sendData, (void *)&td[i]);
    if (rc < 0) {
      fprintf(log, "pthread %d create failed\n", i);
      exit(1);
    }
  }
  pthread_exit(NULL);
}
