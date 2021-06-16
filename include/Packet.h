#ifndef _PACKET_H
#define _PACKET_H

#include "IgnoreError.h"

struct ring {
  struct iovec *rd;
  uint8_t *map;
  struct tpacket_req3 req;
};

#endif
