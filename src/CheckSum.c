#include "../include/CheckSum.h"

unsigned char checksum(unsigned char *buf, int n) {
  unsigned long sum;
  for (sum = 0; n > 0; n--) {
    sum += *(buf++);
  }
  while (sum >> 8) {
    sum = (sum >> 8) + (sum & 0xff);
  }
  return ~sum;
}
