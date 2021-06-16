#include <arpa/inet.h>
#include <getopt.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "../../include/AddressUtility.h"
#include "../../include/CheckSum.h"
#include "../../include/Constant.h"
#include "../../include/HandleError.h"

#define DEFAULT_BUF_SIZE 1024
#define DEFAULT_PORT 6201
#define FILE_STREAM stdout

#define OPT_STR "b::p::"

void parseArgs(int argc, char const *argv[], int *buf_size, int *port);

unsigned long now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

void usage(const char *program) {
  fprintf(stderr,
          "Usage: %s [OPTIONS]\n"
          "Options:\n"
          " -b, --buffer_size\n"
          " -p, --server_port\n"
          "\n",
          program);
  exit(EXIT_FAILURE);
}

struct option long_options[] = {
    {"buffer_size", optional_argument, NULL, 'b'},
    {"server_port", optional_argument, NULL, 'p'},
};

int notChanged(unsigned char *buf, int buf_size) {
  int group_count = buf_size / sizeof(unsigned char);
  unsigned char ch = checksum(buf, group_count);
  return ch == 0 ? 1 : 0;
}

int main(int argc, char const *argv[]) {
  int buf_size = DEFAULT_BUF_SIZE, port = DEFAULT_PORT;
  int errorCount = 0;
  int servSock;
  unsigned long start, end;
  struct sockaddr_in servAddr;
  parseArgs(argc, argv, &buf_size, &port);
  unsigned char buffer[buf_size];
  socklen_t servAddrLen = sizeof(servAddr);

  servSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (servSock < 0) {
    DieWithSystemMessage("socket() failed");
  }
  memset(&servAddr, 0, servAddrLen);
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(port);
  servAddr.sin_addr.s_addr = htons(INADDR_ANY);
  if (bind(servSock, (struct sockaddr *)&servAddr, servAddrLen) < 0) {
    close(servSock);
    DieWithSystemMessage("bind() failed");
  }
  while (1) {
    // fprintf(FILE_STREAM, "server start listening...\n");
    memset(buffer, 0, buf_size);
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    start = now_ns();
    ssize_t len = recvfrom(servSock, buffer, buf_size, 0,
                           (struct sockaddr *)&clntAddr, &clntAddrLen);
    end = now_ns();
    if (len < 0) {
      close(servSock);
      DieWithSystemMessage("recvfrom() failed");
    } else if (len == 0) {
      close(servSock);
      DieWithSystemMessage("recv empty!");
    }
    int correct = notChanged(buffer, buf_size);
    if (!correct) {
      printf("packet lost!\n;");
    }
    char clntIP[50];
    struct sockaddr_in *from = (struct sockaddr_in *)&clntAddr;
    inet_ntop(AF_INET, (void *)&from->sin_addr, clntIP, sizeof(clntIP));
    PrintSocketAddress((struct sockaddr *)&clntAddr, FILE_STREAM);
    fputc('\n', FILE_STREAM);
  }
  close(servSock);
}

void parseArgs(int argc, char const *argv[], int *buf_size, int *port) {
  int opt, optIdx;
  while ((opt = getopt_long(argc, argv, OPT_STR, long_options, &optIdx)) !=
         -1) {
    switch (opt) {
    case 'b':
      *buf_size = atoi(optarg);
      break;
    case 'p':
      *port = atoi(optarg);
      break;
    default:
      usage(argv[0]);
      DieWithUserMessage("opt failed()", "there is no such option!");
    }
  }
}
