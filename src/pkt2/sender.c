#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/if_packet.h>

#include "../../include/AddressUtility.h"
#include "../../include/CheckSum.h"
#include "../../include/Constant.h"
#include "../../include/HandleError.h"
#include "../../include/RandUtility.h"
// #include "../../include/IgnoreError.h"

#define MAX_IP_LENGTH 20
#define MAX_INTERFACE_LANGTH 50
#define MAX_MAC_LENGTH 20
#define FILE_STREAM stderr
#define ETH_HDR_SIZE sizeof(struct ethhdr)
#define IP_HDR_SIZE sizeof(struct iphdr)
#define UDP_HDR_SIZE sizeof(struct udphdr)

#define OPT_STR "r::t::p::i:f:m:"

struct ArgData {
  int repeatTime;
  int threadCount;
  int packetSize;
  char destIP[MAX_IP_LENGTH];
  char destMAC[MAX_MAC_LENGTH];
  char interface[MAX_INTERFACE_LANGTH];
};

struct option longopts[] = {
    {"repeatTime", optional_argument, NULL, 'r'},
    {"threadCount", optional_argument, NULL, 't'},
    {"packetSize", optional_argument, NULL, 'p'},
    {"destIP", required_argument, NULL, 'i'},
    {"interface", required_argument, NULL, 'f'},
    {"destMAC", required_argument, NULL, 'm'},
};

void usage(const char *program) {
  fprintf(stderr,
          "Usage: %s [OPTIONS]\n"
          "Options:\n"
          " -f, --interface\n"
          " -i, --destIP\n"
          " -m, --destMAC\n"
          " -r, --repeatTime\n"
          " -t, --threadCount\n"
          " -p, --packetSize\n"
          "\n",
          program);
  exit(EXIT_FAILURE);
}

void parseArgs(int argc, char const *argv[], struct ArgData *arg) {
  int opt, optIdx;
  while ((opt = getopt_long(argc, argv, OPT_STR, longopts, &optIdx)) != -1) {
    switch (opt) {
    case 'r':
      arg->repeatTime = atoi(optarg);
      break;
    case 't':
      arg->threadCount = atoi(optarg);
      break;
    case 'p':
      arg->packetSize = atoi(optarg);
      break;
    case 'i':
      strncpy(arg->destIP, optarg, MAX_IP_LENGTH);
      break;
    case 'f':
      strncpy(arg->interface, optarg, MAX_INTERFACE_LANGTH);
      break;
    case 'm':
      strncpy(arg->destMAC, optarg, MAX_MAC_LENGTH);
      break;
    default:
      usage(argv[0]);
      DieWithUserMessage("parse args failed!", "no such option!");
    }
  }
}

void fillPacket(unsigned char buf[], int packet_size) {
  int group_count = packet_size / sizeof(unsigned char);
  resetSeed();
  for (int i = 0; i < group_count - 1; i++) {
    buf[i] = (char)randInt(0, 127);
  }
  buf[group_count - 1] = checksum(buf, group_count - 1);
}

void setEthHeader(int sock, unsigned char *buf, int *bufLen,
                  struct ArgData *arg) {
  struct ifreq ifreq_c;
  memset(&ifreq_c, 0, sizeof(ifreq_c));
  strncpy(ifreq_c.ifr_name, arg->interface, IFNAMSIZ - 1);
  if (ioctl(sock, SIOCGIFHWADDR, &ifreq_c) < 0) {
    DieWithSystemMessageClose("ioctl(SIOCGIFHWADDR) failed!", sock);
  }
  struct ethhdr *eth = (struct ethhdr *)buf;
  eth->h_source[0] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
  eth->h_source[1] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
  eth->h_source[2] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
  eth->h_source[3] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
  eth->h_source[4] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
  eth->h_source[5] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);
  char *tok = strtok(arg->destMAC, ":");
  eth->h_dest[0] = (unsigned char)tok[0];
  eth->h_dest[1] = (unsigned char)tok[1];
  eth->h_dest[2] = (unsigned char)tok[2];
  eth->h_dest[3] = (unsigned char)tok[3];
  eth->h_dest[4] = (unsigned char)tok[4];
  eth->h_dest[5] = (unsigned char)tok[5];
  eth->h_proto = htons(ETH_P_IP);
  bufLen += ETH_HDR_SIZE;
}

void setIPHeader(int sock, unsigned char *buf, int *bufLen,
                 struct ArgData *arg) {
  struct ifreq ifreq_ip;
  strncpy(ifreq_ip.ifr_name, arg->interface, IFNAMSIZ - 1);
  if (ioctl(sock, SIOCGIFADDR, &ifreq_ip) < 0) {
    DieWithSystemMessageClose("ioctl(SIOCGIFADDR) failed", sock);
  }
  struct iphdr *iph = (struct iphdr *)(buf + ETH_HDR_SIZE);
  iph->ihl = 5;
  iph->version = 4;
  iph->tos = 16;
  iph->id = htons(10201);
  iph->ttl = 64;
  iph->protocol = 17;
  iph->saddr = inet_addr(
      inet_ntoa((((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr)));
  iph->daddr = inet_addr(arg->destIP);
  *bufLen += IP_HDR_SIZE;
  setUDPHeader(buf, bufLen);
  iph->tot_len = htons(*bufLen - ETH_HDR_SIZE);
  iph->check = htons(checksum(buf + ETH_HDR_SIZE, IP_HDR_SIZE / 2));
}

void setUDPHeader(unsigned char *buf, int *bufLen) {
  struct udphdr *uh = (struct udphdr *)(buf + IP_HDR_SIZE + ETH_HDR_SIZE);
  uh->source = htons(23453);
  uh->dest = htons(23452);
  *bufLen += UDP_HDR_SIZE;
  int dataSize = sizeof(buf) - ETH_HDR_SIZE - IP_HDR_SIZE - UDP_HDR_SIZE;
  fillPacket(buf + ETH_HDR_SIZE + IP_HDR_SIZE + UDP_HDR_SIZE, dataSize);
  uh->check = checksum(buf, bufLen);
  uh->len = htons((*bufLen - IP_HDR_SIZE - ETH_HDR_SIZE));
}

int main(int argc, char const *argv[]) {
  struct ArgData arg;
  parseArgs(argc, argv, &arg);
  unsigned char buf[arg.packetSize];
  memset(buf, 0, sizeof(buf));
  int bufLen = 0;
  int sendSock = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (sendSock < 0) {
    DieWithSystemMessage("socked() failed!");
  }
  struct ifreq ifreq_i, ifreq_r, ifreq_ip;
  memset(&ifreq_i, 0, sizeof(ifreq_i));
  strncpy(ifreq_i.ifr_name, arg.interface, IFNAMSIZ - 1);
  if (ioctl(sendSock, SIOCGIFINDEX, &ifreq_i) < 0) {
    DieWithSystemMessageClose("ioctl(SIOCGIFINDEX) failed!", sendSock);
  }
  setEthHeader(sendSock, buf, &bufLen, &arg);
  setIPHeader(sendSock, buf, &bufLen, &arg);

  struct sockaddr_ll sa_ll;
  sa_ll.sll_ifindex = ifreq_i.ifr_ifindex;
  sa_ll.sll_halen = ETH_ALEN;

  printf("sending...\n");
  for (int i = 0; i < arg.repeatTime; i++) {
    ssize_t len =
        sendto(sendSock, buf, arg.packetSize, 0,
               (const struct sockaddr *)&sa_ll, sizeof(struct sockaddr_ll));
    if (len < 0) {
      DieWithSystemMessageClose("sendto() failed!", sendSock);
    }
  }
}
