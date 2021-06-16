#ifndef _IGNORE_ERROR_H
#define _IGNORE_ERROR_H

#define AF_PACKET 0
#define IPPROTO_RAW 0
#define SIOCGIFINDEX 0
#define SIOCGIFHWADDR 0
#define SIOCGIFADDR 0
#define TPACKET_V3 0
#define SOL_PACKET 0
#define PACKET_VERSION 0
#define PACKET_TX_RING 0
#define PROT_READ 0
#define PROT_WRITE 0
#define MAP_SHARED 0
#define MAP_LOCKED 0
#define MAP_FAILED 0
#define ETH_P_IP 0
#define TPACKET3_HDRLEN 0
#define TP_STATUS_SEND_REQUEST 0

struct tpacket_req3 {
  unsigned int tp_block_size;
  unsigned int tp_block_nr;
  unsigned int tp_frame_size;
  unsigned int tp_frame_nr;
};
typedef unsigned char uint8_t;

struct sockaddr_ll {
  int sll_family;
  int sll_protocol;
  int sll_ifindex;
};

struct tpacket3_hdr {
  int tp_snaplen;
  int tp_len;
  int tp_next_offset;
  int tp_status;
};

struct iphdr {
  int ihl;
  int version;
  int tos;
  int id;
  int ttl;
  int protocol;
  int saddr;
  int daddr;
};

// int ioctl(int, int, struct ifreq *);

// uint8_t *mmap(int *, unsigned int, int, int, int, int);

#endif
