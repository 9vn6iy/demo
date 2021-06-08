#include <stdio.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../../include/constant.h"
#include "../../include/HandleError.h"
#include "../../include/AddressUtility.h"

int main(int argc, char const *argv[]) {
  const char servPort[] = "6201";
  int servSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (servSock < 0) {
    DieWithSystemMessage("socket() failed");
  }

  // config network card
  /*
  struct ifreq ifr;
  memset(&ifr, 0x00, sizeof(ifr));
  char card[] = "ens33";
  strncpy(ifr.ifr_name, card, sizeof(card));
  // strncpy(ifr.ifr_name, "ens33", strlen("ens33"));
  int opt = setsockopt(servSock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
  if (opt < 0) {
    DieWithSystemMessage("setsockopt() failed");
  }
  */

  struct sockaddr_in servAddr;
  socklen_t servAddrLen = sizeof(servAddr);
  bzero(&servAddr, servAddrLen);
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(atoi(servPort));
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  // servAddr.sin_addr.s_addr = htonl("127.0.0.1");
  char buffer[MAX_STRING_LENGTH];

  int res = bind(servSock, (struct sockaddr *)&servAddr, servAddrLen);
  if (res < 0) {
    DieWithSystemMessage("bind() failed");
  }
  while (1) {
    puts("start listening...");
    bzero(buffer, MAX_STRING_LENGTH);
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    printf("server start receiving from client...\n");
    ssize_t numBytesRcvd = recvfrom(servSock, buffer, MAX_STRING_LENGTH, 0, 
        (struct sockaddr *)&clntAddr, &clntAddrLen);
    printf("server end receiving from client...\n");
    if (numBytesRcvd < 0) {
      DieWithSystemMessage("recvfrom() failed");
    }
    fputs("Handling client ", stdout);
    PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
    fputc('\n', stdout);

    // send recv datagram back to the client
    /*
    ssize_t numBytesSent = sendto(servSock, buffer, numBytesRcvd, 0,
        (struct sockaddr *)&clntAddr, sizeof(clntAddr));
    if (numBytesSent < 0) {
      DieWithSystemMessage("sendto() failed");
    } else if (numBytesSent != numBytesRcvd) {
      DieWithUserMessage("sendto()", "sent unexpected number of bytes");
    }
    */
  }
  close(servSock);
  exit(0);
}
