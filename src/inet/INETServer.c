#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../include/AddressUtility.h"
#include "../../include/Constant.h"
#include "../../include/HandleError.h"

#define BUF_SIZE 1024

int main(int argc, char const *argv[]) {
  const char servPort[] = "6201";
  int servSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (servSock < 0) {
    DieWithSystemMessage("socket() failed");
  }
  struct sockaddr_in servAddr;
  socklen_t servAddrLen = sizeof(servAddr);
  bzero(&servAddr, servAddrLen);
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(atoi(servPort));
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  // servAddr.sin_addr.s_addr = htonl("127.0.0.1");
  char buffer[BUF_SIZE];

  int res = bind(servSock, (struct sockaddr *)&servAddr, servAddrLen);
  if (res < 0) {
    DieWithSystemMessage("bind() failed");
  }
  while (1) {
    puts("start listening...");
    bzero(buffer, BUF_SIZE);
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    printf("server start receiving from client...\n");
    ssize_t numBytesRcvd = recvfrom(servSock, buffer, BUF_SIZE, 0,
                                    (struct sockaddr *)&clntAddr, &clntAddrLen);
    printf("server end receiving from client...\n");
    if (numBytesRcvd < 0) {
      DieWithSystemMessage("recvfrom() failed");
    }
    fputs("Handling client ", stdout);
    PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
    fputc('\n', stdout);
  }
  close(servSock);
  exit(0);
}
