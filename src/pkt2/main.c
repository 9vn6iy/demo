#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  printf("eth = %zd\n", sizeof(struct ethhdr));
  printf("ip = %zd\n", sizeof(struct iphdr));
  printf("udp = %zd\n", sizeof(struct udphdr));
  return 0;
}
