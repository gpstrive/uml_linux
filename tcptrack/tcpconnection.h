#ifndef TCPCONNECTON_H
#define TCPCONNECTON_H
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#define TCPCONN_TABLE_SIZE (1<<20)
#define TCPCONN_TABLE_MASK (TCPCONN_TABLE_SIZE-1)
#define TCP_STATE_NONE 0
#define TCP_STATE_SYN_SYNACK    1 // initial SYN sent, waiting for SYN ACK
#define TCP_STATE_SYNACK_ACK 2 // SYN&ACK response sent, waiting for ACK
#define TCP_STATE_UP    3 // SYNACK response sent
#define TCP_STATE_FIN_FINACK 4
#define TCP_STATE_CLOSED 5
#define TCP_STATE_RESET 6
#define TCP_STATE_UNKOWN 7
#define TCP_STATE_MAX 8
#ifndef NIPQUAD
#define NIPQUAD(addr) \
    ((u8 *)&(addr))[0],\
    ((u8 *)&(addr))[1],\
    ((u8 *)&(addr))[2],\
    ((u8 *)&(addr))[3]
#endif

typedef unsigned char u8;
typedef unsigned short  u16;
typedef unsigned int  u32;
#define SECS *1
#define MINS * 60 SECS
#define HOURS * 60 MINS
#define DAYS * 24 HOURS
 
static unsigned int tcp_timeouts[TCP_STATE_MAX] = {
    [TCP_STATE_SYN_SYNACK] = 2 MINS,
    [TCP_STATE_SYNACK_ACK] = 60 SECS,
    [TCP_STATE_UP]         = 5 DAYS,
    [TCP_STATE_FIN_FINACK] = 2 MINS,
    [TCP_STATE_CLOSED]     = 10 SECS,
    [TCP_STATE_RESET]      = 10 SECS,
#if 0
     [TCP_CONNTRACK_SYN_SENT]    = 2 MINS,
     [TCP_CONNTRACK_SYN_RECV]    = 60 SECS,
     [TCP_CONNTRACK_ESTABLISHED] = 5 DAYS,
     [TCP_CONNTRACK_FIN_WAIT]    = 2 MINS,
     [TCP_CONNTRACK_CLOSE_WAIT]  = 60 SECS,
     [TCP_CONNTRACK_LAST_ACK]    = 30 SECS,
     [TCP_CONNTRACK_TIME_WAIT]   = 2 MINS,
     [TCP_CONNTRACK_CLOSE]       = 10 SECS,
     [TCP_CONNTRACK_SYN_SENT2]   = 2 MINS,
 /* RFC1122 says the R2 limit should be at least 100 seconds.
    78    Linux uses 15 packets as limit, which corresponds
    79    to ~13-30min depending on RTO. */
     [TCP_CONNTRACK_RETRANS]     = 5 MINS,
     [TCP_CONNTRACK_UNACK]       = 5 MINS,
#endif 
 };


typedef struct tcpconnection_t{
	u32 srcip;
	u32 dstip;
	u16 srcport;
	u16 dstport;
	//  TCP_STATE_* values reflecting the connection's state.
	u8 state;
	//  true if this connection is closed and no more traffic
	// is expected for it.
#define RECV_FINACK_FROM_DST 0x02
#define RECV_FINACK_FROM_SRC 0x04
	u8 isFinished;
	//get idle time 
	u16 last_pkt_timestamp;
    u32 finack_from_src_seq;
    u32 finack_from_dst_seq;
	u32 inBps;
	u32 inpps;
	u32 outBps;
	u32 outpps;
}tcpconnection;
	
typedef struct _pkt_t{
    u8 *l3_hdr;
    u8 *l4_hdr;
    u16 tot_len;
}_pkt;
#endif
