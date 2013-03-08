#include <sys/types.h>       // socket
#include <sys/socket.h>      // socket
#include <sys/ioctl.h>       // ioctl
#include <net/if.h>          // ifreq 
#include <string.h>          // strcpy
#include <stdio.h>           // printf
#include <linux/if_packet.h> // sockaddr_ll
#include <net/ethernet.h>


#define BOOL   unsigned char
#define U8     unsigned char
#define U16    unsigned short 
#define TRUE   1
#define FALSE  0
#define PACKET_SIZE 2000
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned char      u8;
typedef signed char        s8;

static int fd;
static unsigned char if_mac[6];
static int if_index;

U8  gBuf[2000] = {0};
U8  gTxBuf[PACKET_SIZE];
U16 gLen=0;

const char dest_mac_addr[6] = {0x00, 0x90, 0x0b, 0x13, 0xb8, 0x36};
static int build_mac_hdr(struct ethhdr *eh, u16 protocol) {
	char source[6] = {0x00, 0x90, 0x0b, 0x1e, 0xe1, 0xa6};

	memcpy(eh->h_dest, dest_mac_addr, 6);
	memcpy(eh->h_source, source, 6);
	eh->h_proto = htons(protocol);

	return 0;
}
u16 checksum(char *buf, int len)
{
    u32 csum = 0;
    u16 *word = (u16 *)buf;
    while (len > 1) {
        csum += *word++;
        len -= 2;
    }

    if (len)
        csum += *(u8 *)word;

    csum = (csum >> 16) + (csum & 0XFFFF);
    csum += (csum >> 16);
    return (u16)~csum;
}
static int build_ipv4_hdr(struct iphdr *ih, u8 protocol, u32 saddr, u32 daddr, u8 *content, int length) {
	ih->ihl = 5;
	ih->version = 4;
	ih->tos = 0;
	ih->tot_len = htons(length + 20);
	ih->id = 0;
	ih->frag_off = 0;
	ih->ttl = 64;
	ih->protocol = protocol;
	ih->saddr = saddr;
	ih->daddr = daddr;
	ih->check = checksum((char *)ih, 20);

	return length + 20;
}

static int build_udp_hdr(struct udphdr *uh, u16 sport, u16 dport, u8 *content, u32 length) {
	uh->source = htons(sport);
	uh->dest = htons(dport);
	uh->len = htons(length);
	uh->check = checksum((char*)content, length);

	return length + 8;
}
 void start_send(struct _raw_option *op, int sock) {
    int forever = 0;
    unsigned long i = 0;
    int len = 0;
    int ret = 0;
    char buffer[1024];
    char seq[256] = "";
    //struct sockaddr_ll addr;
    //memcpy(&addr, &op->addr, sizeof(addr));


	if (op->count == 0)
		forever = 1;

	cs.pkts = 0;
	cs.bytes = 0;
	os.bytes = 0;
	os.pkts = 0;
	running = 1;

#ifdef BUILD_SEND_PACKET_JUST_ONCE
	memset(seq, 0, sizeof(seq));
	snprintf(seq, sizeof(seq) - 1, "sequence = %lu", i);
	//printf("seq: %s len: %d\n", seq, strlen(seq));
	//ultoa(i, seq, 10);
	len = build_ipv4_pkt(buffer, sizeof(buffer), seq, strlen(seq));
#endif

	while(running && (forever || op->count > 0)) {
#ifndef BUILD_SEND_PACKET_JUST_ONCE
		memset(seq, 0, sizeof(seq));
		snprintf(seq, sizeof(seq) - 1, "sequence no %09lu", i);
		//printf("seq: %s len: %d\n", seq, strlen(seq));
		//ultoa(i, seq, 10);
		len = build_ipv4_pkt(buffer, sizeof(buffer), seq, strlen(seq));
#endif

		ret = sendto(sock, buffer, len, 0, (const struct sockaddr *)&(op->addr), sizeof(op->addr));
		if(ret < 0) {
            perror("sendto");
            printf("sll_family= %hu sll_protocol= %hu sll_ifindex= %d sll_hatype=%hu sll_pkttype=%u sll_halen=%u sll_addr: %x:%x:%x:%x:%x:%x\n",\
                        op->addr.sll_family, op->addr.sll_protocol, op->addr.sll_ifindex, op->addr.sll_hatype, (u32)op->addr.sll_pkttype, \
                        (u32)op->addr.sll_halen, op->addr.sll_addr[0], op->addr.sll_addr[1],op->addr.sll_addr[2],op->addr.sll_addr[3],\
                        op->addr.sll_addr[4],op->addr.sll_addr[5]);
			break;
		}

	}
}

BOOL InitEtherNetIf(void)
{
    struct ifreq req;

    if ( (fd = socket(PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) ) ) < 0 )
    {                         
        printf( "failed to create raw socket!\n" );
        return FALSE;
    }

    strcpy( req.ifr_name, "eth1" );
    if ( ioctl( fd, SIOCGIFFLAGS, &req ) < 0 )
    {
        printf( "failed to do ioctl!" );
        return FALSE;
    }

    req.ifr_flags |= IFF_PROMISC;

    if ( ioctl( fd, SIOCSIFFLAGS, &req ) < 0 )
    {
        printf( "failed to set eth0 into promisc mode!" );
        return FALSE;
    }

    if ( ioctl( fd, SIOCGIFHWADDR, &req ) < 0 )
    {
        printf( "failed to get IF hw address!" );
        return FALSE;
    }

    memcpy( if_mac, req.ifr_hwaddr.sa_data, sizeof(if_mac) );

    if ( ioctl( fd, SIOCGIFINDEX, &req ) < 0 )
    {
        printf( "failed to get IF hw address!" );
        return FALSE;
    }

    if_index = req.ifr_ifindex;
    return TRUE;
}



BOOL GetPacket(U8 *buf, U16 *len) 
{
    int length = 0;

    length = recvfrom( fd, buf, 2000, 0, NULL, NULL );
    if ( length < 0 )
    {
        return 0;
    } 
    else
    {
        *len = length;
        return 1;
    }
}



BOOL SendPacket(U8 *buf, U16 len)
{
    struct sockaddr_ll link;
    link.sll_ifindex = if_index;

    memcpy( link.sll_addr, buf, link.sll_halen );

    if ( sendto( fd, buf, len, 0, (struct sockaddr *)&link, sizeof(link) ) < 0 )
    {
        printf( "failed to send to RAW socket!\r\n" );   
        return 0;
    }
    return 1;
}



BOOL GetMacAddress(U8 *mac)
{
    memcpy(mac, if_mac, sizeof(if_mac));
    return TRUE;
}



void DispalyBuf(U8 *buf,U16 size)
{
    int i;
    for(i=0;i<size;i++)
    {      
        if((0==i%16)&&(i>0)) printf("\n");
        printf("%02x ",buf[i]); 
    }
    printf("\n");
}



int main(void)
{
    BOOL ret;   

    for(ret =0;ret<PACKET_SIZE;ret++) gTxBuf[ret]=ret;

    if(InitEtherNetIf())
    {
        printf("send:\n");
        DispalyBuf(gTxBuf,PACKET_SIZE);
        SendPacket(gTxBuf,PACKET_SIZE);
        while(1)
        {
            if(GetPacket(gBuf,&gLen))   DispalyBuf(gBuf,gLen);
        } 
    }
}


