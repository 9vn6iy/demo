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

#include "../../include/Constant.h"
#include "../../include/HandleError.h"

#define REPEAT_TIME 1000
// #define BUF_SIZE 64
// 548 * 8
#define PACKET_SIZE 1024
#define UDP_HEADER_SIZE 64
#define SERVER_PORT "6201"
// #define SERVER_IP "10.252.152.130"
// #define SERVER_IP "10.126.82.28"
// #define SERVER_IP "10.178.1.1"
#define SERVER_IP "10.178.21.11"

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
  const char *serverIP;
  int packetSize;
};

int initPort = 6201;

void *sendData(void *arg) {
  struct ThreadData *t = (struct ThreadData *)arg;
  time_t start, end;
  clock_t st, ed;
  double durationSec = 0.0, totalDurationSec = 0.0;
  double avgDurationSec = 0.0;
  char data[t->packetSize];
  long dataSize, totalDataSize = 0L;
  // size_t dataLen = strlen(data);
  size_t dataLen = sizeof(data);
  fill(data, t->packetSize);
  int clntSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (clntSock < 0) {
    DieWithSystemMessage("socket() failed");
  }
  struct sockaddr_in servAddr;
  socklen_t servAddrLen = sizeof(servAddr);
  memset(&servAddr, 0, servAddrLen);
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET, t->serverIP, &servAddr.sin_addr);
  servAddr.sin_port = htons(initPort);
  for (int i = 0; i < 50; i++) {
    start = time(NULL);
    st = clock();
    dataSize = 0;
    for (int j = 0; j < t->repeatTime; j++) {
      ssize_t len = sendto(clntSock, data, dataLen, 0,
                           (struct sockaddr *)&servAddr, servAddrLen);
      if (len < 0) {
        DieWithSystemMessage("sendto() failed");
      } else if (len != dataLen) {
        DieWithUserMessage("sendto() error", "sent unexpected number of bytes");
      }
      dataSize += len;
    }
    end = time(NULL);
    ed = clock();
    durationSec = (end - start);
    // durationSec = (double)(ed - st) / CLOCKS_PER_SEC;
    totalDurationSec += durationSec;
    totalDataSize += dataSize;
  }
  close(clntSock);
  avgDurationSec = totalDurationSec / 50.0;
  fprintf(t->log, "thread %2d total duration = %f sec, ", t->id,
          avgDurationSec);
  // printf("total throught = %f bit/sec\n", totalDataSize / totalDurationSec);
  fprintf(t->log, "bitrate= %f mbps\n",
          (totalDataSize / totalDurationSec) / 1048576);
  return NULL;
}

// <repeat time, thread count>
int main(int argc, char const *argv[]) {
  int repeatTime, threadCount;
  char serverIP[50];
  int packetSize;
  if (argc < 3) {
    repeatTime = 4000;
    threadCount = 1;
  } else {
    repeatTime = atoi(argv[2]);
    threadCount = atoi(argv[3]);
    strncpy(serverIP, argv[1], 50);
    packetSize = atoi(argv[4]);
  }
  pthread_t threads[threadCount];
  struct ThreadData td[threadCount];
  FILE *log = fopen("log.txt", "w");
  fprintf(log, "repeat count = %s, thread count = %s\n", argv[1], argv[2]);
  for (int i = 0; i < threadCount; i++) {
    td[i].id = i;
    td[i].log = log;
    td[i].repeatTime = repeatTime;
    td[i].serverIP = serverIP;
    td[i].packetSize = packetSize;
    fprintf(log, "create thread %d\n", i);
    int rc = pthread_create(&threads[i], NULL, sendData, (void *)&td[i]);
    if (rc < 0) {
      fprintf(log, "pthread %d create failed\n", i);
      exit(1);
    }
  }
  pthread_exit(NULL);
}
