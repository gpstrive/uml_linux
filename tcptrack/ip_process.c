#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include "tcpconnection.h"
int ip_process(_pkt *pkt){
    struct iphdr *iphdr;
    iphdr = (struct iphdr *)pkt->l3_hdr;
	//printf("iphdr->protocol=%d\n",iphdr->protocol);
    if(iphdr->protocol == IPPROTO_TCP)
        tcp_process(pkt);
#if 0
    else if (ip_hdr->ip_p = ntohs(IPPROTO_UDP))
        udp_process(packet);
#endif
    else 
      return 1;
    return 0;
}
