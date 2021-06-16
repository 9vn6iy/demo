/* Note: run this program as root user
 * Author:Subodh Saxena
 */
#include <errno.h>
#include <getopt.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <linux/if_packet.h>

#include <arpa/inet.h>

#include "../../include/Constant.h"
#include "../../include/HandleError.h"
#include "../../include/RandUtility.h"

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
      printf("parse args failed!\n");
      exit(1);
    }
  }
}

struct ifreq ifreq_c, ifreq_i, ifreq_ip;
int sock_raw;
unsigned char *sendbuff;
struct ArgData arg;

int total_len = 0, send_len;

void get_eth_index() {
  memset(&ifreq_i, 0, sizeof(ifreq_i));
  strncpy(ifreq_i.ifr_name, arg.interface, IFNAMSIZ - 1);

  if ((ioctl(sock_raw, SIOCGIFINDEX, &ifreq_i)) < 0)
    printf("error in index ioctl reading");

  printf("index=%d\n", ifreq_i.ifr_ifindex);
}

void get_mac() {
  memset(&ifreq_c, 0, sizeof(ifreq_c));
  strncpy(ifreq_c.ifr_name, arg.interface, IFNAMSIZ - 1);

  if ((ioctl(sock_raw, SIOCGIFHWADDR, &ifreq_c)) < 0)
    printf("error in SIOCGIFHWADDR ioctl reading");

  printf("Mac= %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
         (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]),
         (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]),
         (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]),
         (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]),
         (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]),
         (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]));

  printf("ethernet packaging start ... \n");

  struct ethhdr *eth = (struct ethhdr *)(sendbuff);
  eth->h_source[0] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
  eth->h_source[1] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
  eth->h_source[2] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
  eth->h_source[3] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
  eth->h_source[4] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
  eth->h_source[5] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);

  printf("dest MAC=%s\n", arg.destMAC);
  char *tok = strtok(arg.destMAC, ":");
  int i = 0;
  while (tok != NULL) {
    sscanf(tok, "%X", &eth->h_dest[i++]);
    tok = strtok(NULL, ":");
  }
  eth->h_proto = htons(ETH_P_IP);

  printf("dest Mac= %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_dest[0],
         eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4],
         eth->h_dest[5]);
  printf("ethernet packaging done.\n");

  total_len += sizeof(struct ethhdr);
}

void get_data() {
  sendbuff[total_len++] = 0xAA;
  sendbuff[total_len++] = 0xBB;
  sendbuff[total_len++] = 0xCC;
  sendbuff[total_len++] = 0xDD;
  sendbuff[total_len++] = 0xEE;
}

void get_udp() {
  struct udphdr *uh = (struct udphdr *)(sendbuff + sizeof(struct iphdr) +
                                        sizeof(struct ethhdr));

  uh->source = htons(23451);
  uh->dest = htons(23452);
  uh->check = 0;

  total_len += sizeof(struct udphdr);
  get_data();
  uh->len = htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr)));
}

unsigned short checksum(unsigned short *buff, int _16bitword) {
  unsigned long sum;
  for (sum = 0; _16bitword > 0; _16bitword--)
    sum += htons(*(buff)++);
  do {
    sum = ((sum >> 16) + (sum & 0xFFFF));
  } while (sum & 0xFFFF0000);

  return (~sum);
}

void get_ip() {
  memset(&ifreq_ip, 0, sizeof(ifreq_ip));
  strncpy(ifreq_ip.ifr_name, arg.interface, IFNAMSIZ - 1);
  if (ioctl(sock_raw, SIOCGIFADDR, &ifreq_ip) < 0) {
    printf("error in SIOCGIFADDR \n");
  }

  printf("%s\n",
         inet_ntoa((((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr)));

  /****** OR
          int i;
          for(i=0;i<14;i++)
          printf("%d\n",(unsigned char)ifreq_ip.ifr_addr.sa_data[i]); ******/

  struct iphdr *iph = (struct iphdr *)(sendbuff + sizeof(struct ethhdr));
  iph->ihl = 5;
  iph->version = 4;
  iph->tos = 16;
  iph->id = htons(10201);
  iph->ttl = 64;
  iph->protocol = 17;
  iph->saddr = inet_addr(
      inet_ntoa((((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr)));
  iph->daddr = inet_addr(arg.destIP);
  total_len += sizeof(struct iphdr);
  get_udp();

  iph->tot_len = htons(total_len - sizeof(struct ethhdr));
  iph->check =
      htons(checksum((unsigned short *)(sendbuff + sizeof(struct ethhdr)),
                     (sizeof(struct iphdr) / 2)));
}

int main(int argc, char const *argv[]) {
  parseArgs(argc, argv, &arg);
  sock_raw = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (sock_raw == -1)
    printf("error in socket");
  int v = TPACKET_V3;
  if (setsockopt(sock_raw, SOL_SOCKET, PACKET_VERSION, &v, sizeof(v)) < 0) {
    printf("error in set");
  }

  sendbuff = (unsigned char *)malloc(arg.packetSize);
  memset(sendbuff, 0, 64);

  get_eth_index(); // interface number
  get_mac();
  get_ip();

  struct sockaddr_ll sadr_ll;
  sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
  sadr_ll.sll_halen = ETH_ALEN;
  char *tok = strtok(arg.destMAC, ":");
  int i = 0;
  while (tok != NULL) {
    sscanf(tok, "%X", &sadr_ll.sll_addr[i++]);
    tok = strtok(NULL, ":");
  }
  printf("sending...\n");
  for (int i = 0; i < arg.repeatTime; i++) {
    send_len =
        sendto(sock_raw, sendbuff, 64, 0, (const struct sockaddr *)&sadr_ll,
               sizeof(struct sockaddr_ll));
    if (send_len < 0) {
      printf("error in sending....sendlen=%d....errno=%d\n", send_len, errno);
      return -1;
    }
  }
}
