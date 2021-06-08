#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <time.h>

#include <linux/if_packet.h>

#include <arpa/inet.h>
struct ifreq ifreq_c, ifreq_i,
    ifreq_ip; /// for each ioctl keep diffrent ifreq structure otherwise error
              /// may come in sending(sendto )
int sock_raw;
unsigned char *sendbuff;

#define DESTMAC0 0xd0
#define DESTMAC1 0x67
#define DESTMAC2 0xe5
#define DESTMAC3 0x12
#define DESTMAC4 0x6f
#define DESTMAC5 0x8f

// #define destination_ip 10.240.253.10
// #define destination_ip 127.0.0.1
#define destination_ip 10.252.152.130
// #define destination_ip 192.168.0.130

#define REPEAT_TIME 100000
#define PACKET_SIZE 1024
#define BUF_SIZE 64

int total_len = 0, send_len;

void get_eth_index() {
  memset(&ifreq_i, 0, sizeof(ifreq_i));
  // strncpy(ifreq_i.ifr_name,"wlan0",IFNAMSIZ-1);
  strncpy(ifreq_i.ifr_name, "ens33", IFNAMSIZ - 1);

  if ((ioctl(sock_raw, SIOCGIFINDEX, &ifreq_i)) < 0)
    printf("error in index ioctl reading");

  printf("index=%d\n", ifreq_i.ifr_ifindex);
}

void get_mac() {
  memset(&ifreq_c, 0, sizeof(ifreq_c));
  // strncpy(ifreq_c.ifr_name,"wlan0",IFNAMSIZ-1);
  strncpy(ifreq_c.ifr_name, "ens33", IFNAMSIZ - 1);

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

  eth->h_dest[0] = DESTMAC0;
  eth->h_dest[1] = DESTMAC1;
  eth->h_dest[2] = DESTMAC2;
  eth->h_dest[3] = DESTMAC3;
  eth->h_dest[4] = DESTMAC4;
  eth->h_dest[5] = DESTMAC5;

  eth->h_proto = htons(ETH_P_IP); // 0x800

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
  // strncpy(ifreq_ip.ifr_name,"wlan0",IFNAMSIZ-1);
  strncpy(ifreq_ip.ifr_name, "ens33", IFNAMSIZ - 1);
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
  iph->daddr = inet_addr("destination_ip"); // put destination IP address
  total_len += sizeof(struct iphdr);
  get_udp();

  iph->tot_len = htons(total_len - sizeof(struct ethhdr));
  iph->check =
      htons(checksum((unsigned short *)(sendbuff + sizeof(struct ethhdr)),
                     (sizeof(struct iphdr) / 2)));
}

int main() {
  clock_t start, end;
  double durationSec, totalDurationSec = 0;
  double avgDurationSec = 0;
  long dataSize, totalDataSize = 0L;
  sock_raw = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (sock_raw == -1)
    printf("error in socket");

  int v = TPACKET_V3;
  int opt = setsockopt(sock_raw, SOL_PACKET, PACKET_VERSION, &v, sizeof(v));
  if (opt < 0) {
    printf("setsockopt() failed\n");
    exit(1);
  }
  sendbuff = (unsigned char *)malloc(
      PACKET_SIZE); // increase in case of large data.Here data is --> AA  BB  CC  DD  EE
  memset(sendbuff, 0, PACKET_SIZE);

  get_eth_index(); // interface number
  get_mac();
  get_ip();

  struct sockaddr_ll sadr_ll;
  sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
  sadr_ll.sll_halen = ETH_ALEN;
  sadr_ll.sll_addr[0] = DESTMAC0;
  sadr_ll.sll_addr[1] = DESTMAC1;
  sadr_ll.sll_addr[2] = DESTMAC2;
  sadr_ll.sll_addr[3] = DESTMAC3;
  sadr_ll.sll_addr[4] = DESTMAC4;
  sadr_ll.sll_addr[5] = DESTMAC5;

  printf("sending...\n");
  for (int j = 0; j < 50; j++) { 
    start = clock();
    dataSize = total_len;
    for (int i = 0; i < REPEAT_TIME; i++) {
      send_len = sendto(sock_raw, sendbuff, PACKET_SIZE, 0, (const struct sockaddr *)&sadr_ll,
                 sizeof(struct sockaddr_ll));
      if (send_len < 0) {
        printf("error in sending....sendlen=%d....errno=%d\n", send_len, errno);
        exit(1);
      }  else if (send_len != PACKET_SIZE) {
        printf("send_len != PACKET_SIZE\n");
        exit(1);
      }
    }
    dataSize += PACKET_SIZE;
    end = clock();
    totalDataSize += dataSize;
    durationSec = (end - start) / CLOCKS_PER_SEC;
    printf("duration: %f sec, ", durationSec);
    printf("bitrate: %f mbps\n", dataSize / durationSec / 1000000);
    totalDurationSec += durationSec;
  }
  avgDurationSec = totalDurationSec / 50.0;
  printf("avg duration: %f sec\n", avgDurationSec);
  printf("avg bitrate: %f mbps\n", totalDataSize / totalDurationSec / 1000000);
}
