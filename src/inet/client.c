#include <arpa/inet.h>
#include <getopt.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "../../include/Constant.h"
#include "../../include/HandleError.h"
#include "../../include/RandUtility.h"
#include "../../include/CheckSum.h"

#define DEFAULT_PACKET_SIZE 1024
#define DEFAULT_THREAD_COUNT 1
#define DEFAULT_REPEAT_TIME 20
#define DEFAULT_PORT 6201
#define FILE_STREAM stdout

#define OPT_STR "i:r::t::s::"

struct ThreadData {
  int id;
  int repeatTime;
  const char *servIP;
  int packetSize;
};

void fillPacket(unsigned char buf[], int packet_size) {
  int group_count = packet_size / sizeof(unsigned char);
  resetSeed();
  for (int i = 0; i < group_count - 1; i++) {
    buf[i] = (char)randInt(0, 127);
  }
  buf[group_count - 1] = checksum(buf, group_count - 1);
}

struct option long_options[] = {
    {"destination_ip", required_argument, NULL, 'i'},
    {"packet_size", optional_argument, NULL, 's'},
    {"repeat_time", optional_argument, NULL, 'r'},
    {"thread_count", optional_argument, NULL, 't'},
};

unsigned long now_ns(void);

void parseArgs(int argc, char const *argv[], int *repeatTime, int *threadCount,
               int *packetSize, char *serverIP);

void *handleThread(void *arg);

void usage(const char *program) {
  fprintf(stderr,
          "Usage: %s [OPTIONS]\n"
          "Options:\n"
          " -i, --destination_ip\n"
          " -s, --packet_size\n"
          " -r, --repeat_time\n"
          " -t, --thread_count\n"
          "\n",
          program);
  exit(EXIT_FAILURE);
}

// <servIP, repeatTime, threadCount, packetSize>
int main(int argc, char const *argv[]) {
  int repeatTime = DEFAULT_REPEAT_TIME;
  int threadCount = DEFAULT_THREAD_COUNT;
  int packetSize = DEFAULT_PACKET_SIZE;
  char serverIP[50];
  parseArgs(argc, argv, &repeatTime, &threadCount, &packetSize, serverIP);
  pthread_t threads[threadCount];
  struct ThreadData td[threadCount];
  fprintf(FILE_STREAM,
          "=====================starting client=====================\n");
  fprintf(FILE_STREAM, "destination ip = %s\n", serverIP);
  fprintf(FILE_STREAM, "repeat time = %d\n", repeatTime);
  fprintf(FILE_STREAM, "thread count = %d\n", threadCount);
  fprintf(FILE_STREAM, "packet size = %d\n", packetSize);
  for (int i = 0; i < threadCount; i++) {
    td[i].id = i;
    td[i].repeatTime = repeatTime;
    td[i].servIP = serverIP;
    td[i].packetSize = packetSize;
    fprintf(FILE_STREAM, "create thread %d\n", i);
    if (pthread_create(&threads[i], NULL, handleThread, (void *)&td[i])) {
      DieWithSystemMessage("create pthread failed");
    }
  }
  fprintf(FILE_STREAM,
          "=====================closing client=====================\n");
  pthread_exit(NULL);
}

void *handleThread(void *arg) {
  struct ThreadData *t = (struct ThreadData *)arg;
  int clntSock;
  struct sockaddr_in servAddr;
  socklen_t servAddrLen = sizeof(servAddr);
  unsigned long start, end;
  unsigned long timeNs;
  double timeSec;
  double avgTimeNs;
  unsigned long dataSize_b;
  double dataSize_mb;
  unsigned char data[t->packetSize];

  if ((clntSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DieWithSystemMessage("socket() failed");
  }
  memset(&servAddr, 0, servAddrLen);
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = inet_addr(t->servIP);
  servAddr.sin_port = htons(DEFAULT_PORT);
  start = now_ns();
  dataSize_b = 0;
  for (int i = 0; i < t->repeatTime; i++) {
    fillPacket(data, sizeof(data));
    ssize_t len = sendto(clntSock, data, sizeof(data), 0,
                         (struct sockaddr *)&servAddr, servAddrLen);
    if (len < 0) {
      close(clntSock);
      DieWithSystemMessage("sendto() failed");
    } else if (len != sizeof(data)) {
      close(clntSock);
      DieWithUserMessage("sendto() error", "send unexpected number of bytes");
    }
    dataSize_b += 8 * len;
  }
  end = now_ns();
  timeNs = end - start;
  timeSec = timeNs / 1e9;
  dataSize_mb = dataSize_b / 1e6;
  fprintf(FILE_STREAM, "thread %2d: ", t->id);
  fprintf(FILE_STREAM, "time: %f sec", timeSec);
  fprintf(FILE_STREAM, ", bitrate = %f mbps\n", dataSize_mb / timeSec);
  close(clntSock);
  return NULL;
}

unsigned long now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

void parseArgs(int argc, char const *argv[], int *repeatTime, int *threadCount,
               int *packetSize, char *serverIP) {
  int opt;
  int optIdx;
  while ((opt = getopt_long(argc, argv, OPT_STR, long_options, &optIdx)) !=
         -1) {
    switch (opt) {
    case 'i':
      strcpy(serverIP, optarg);
      break;
    case 'r':
      *repeatTime = atoi(optarg);
      break;
    case 't':
      *threadCount = atoi(optarg);
      break;
    case 's':
      *packetSize = atoi(optarg);
      break;
    default:
      usage(argv[0]);
      DieWithUserMessage("opt failed()", "there is no such option!");
    }
  }
}
