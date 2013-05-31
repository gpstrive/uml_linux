#include <netinet/ip.h>
#include "tcpconnection.h"
#include "tcp.h"
tcpconnection *tcpconn_table;
int init_tcpconn_table(){
	tcpconn_table=(tcpconnection *)malloc(sizeof(tcpconnection)*TCPCONN_TABLE_SIZE);
	if(tcpconn_table == NULL){
		printf("tcpconn_table allocation error\n");
		return -1;
	}
	return 0;
}
u32 hash(u32 srcip,u32 dstip,u16 srcport,u16 dstport){
	return (srcip ^ dstip ^ srcport ^ dstport);
}
static void inline 
set_in_tcpconn(tcpconnection *ptcpconn,u32 srcip,u32 dstip,u16 srcport,u16 dstport,u8 state,u16 Bytes){
	ptcpconn->srcip = srcip;
	ptcpconn->dstip = dstip;
	ptcpconn->srcport = srcport;
	ptcpconn->dstport = dstport;
	ptcpconn->state = state;
	ptcpconn->last_pkt_timestamp = time(NULL);
	ptcpconn->inpps ++;
	ptcpconn->inBps += Bytes ;
}
static void inline 
set_out_tcpconn(tcpconnection *ptcpconn,u32 srcip,u32 dstip,u16 srcport,u16 dstport,u8 state,u16 Bytes){
	ptcpconn->srcip = dstip;
	ptcpconn->dstip = srcip;
	ptcpconn->srcport = dstport;
	ptcpconn->dstport = srcport;
	ptcpconn->state = state;
	ptcpconn->last_pkt_timestamp = time(NULL);
	ptcpconn->outpps ++;
	ptcpconn->outBps += Bytes ;
}
static void inline 
update_in_tcpconn(tcpconnection *ptcpconn,u16 Bytes){
	ptcpconn->last_pkt_timestamp = time(NULL);
	ptcpconn->inpps ++;
	ptcpconn->inBps += Bytes ;
}
static void inline 
update_out_tcpconn(tcpconnection *ptcpconn,u16 Bytes){
	ptcpconn->last_pkt_timestamp = time(NULL);
	ptcpconn->outpps ++;
	ptcpconn->outBps += Bytes ;
}
static int inline 
match(tcpconnection *ptcpconn,u32 srcip,u32 dstip,u16 srcport,u16 dstport){
    return !(ptcpconn->srcip ^ srcip || ptcpconn->dstip ^ dstip || 
                ptcpconn->srcport ^ srcport || ptcpconn->dstport ^ dstport);
}
    

int tcp_process(_pkt *pkt){

    struct iphdr *iphdr;
    struct tcphdr *tcphdr;
    u32 srcip,dstip;
    u32 hashvalue;
    u16 srcport,dstport,tot_len;
    u8 iphdr_len;
    u16 payload_len;
	tcpconnection *ptcpconn;

    iphdr = (struct iphdr *)pkt->l3_hdr;
    srcip = iphdr->saddr;
    dstip = iphdr->daddr;
    iphdr_len = iphdr->ihl << 2;
    tot_len = pkt->tot_len;
    pkt->l4_hdr = pkt->l3_hdr + iphdr_len;

    tcphdr = (struct tcphdr *)(pkt->l4_hdr);
	srcport = ntohs(tcphdr->source);
	dstport = ntohs(tcphdr->dest);
	hashvalue = hash(srcip,dstip,srcport,dstport)&TCPCONN_TABLE_MASK;
	ptcpconn=&tcpconn_table[hashvalue];
    payload_len= ntohs(iphdr->tot_len) - iphdr_len - (tcphdr->doff << 2);
	//reference linux
#if 0
	if(unlikely(tcphdr->syn && tcphdr->fin)){
		return ;
	}
	if(unlikely(tcphdr->syn && tcphdr->rst)){
		return ;
	}
#endif 
	
   // printf("hashvalue=%d,%d.%d.%d.%d,srcport=%d\n",hashvalue,NIPQUAD(ptcpconn->dstip),ptcpconn->dstport);
	if(ptcpconn->dstip == 0 ){
		//new connection
		if(tcphdr->syn ){
            if(tcphdr->doff <= 5){
                printf("incomplete syn or synack \n");
                return 0 ;
            }
			if(!tcphdr->ack){
				set_in_tcpconn(ptcpconn,srcip,dstip,srcport,dstport,TCP_STATE_SYN_SYNACK,tot_len);
				return 0 ;
			}
			else {
				set_out_tcpconn(ptcpconn,srcip,dstip,srcport,dstport,TCP_STATE_SYNACK_ACK,tot_len);
				return 0 ;
			}
				
		}
		// only guess
		else {
			if(tcphdr->ece || tcphdr->fin || tcphdr->rst)
				return 0;
			else{
                /* Currently a TCPConnection expects to be built from a packet
                   that is going from the client to the server. This is because
                   the client initiates the connection and TCPConnection
                   was originally coded to only accept the initial SYN packet
                   to its constructor. At some point this logic may be
                   moved into the TCPConnection constructor.

                   crude way to guess at which end is the client:
                   whichever end has the lowest port number.
                   TODO: can this cli/server guessing be made more intelligent?
                 */

                if(srcport > dstport){
                    // this packet might be the one we saw that went from client->server.
                    set_in_tcpconn(ptcpconn,srcip,dstip,srcport,dstport,TCP_STATE_UNKOWN,tot_len);
                    ptcpconn->finack_from_dst_seq = ntohl(tcphdr->ack_seq) ;
                    printf("from client to server expect sequence=%x\n",ptcpconn->finack_from_dst_seq);
                    return 0 ;
                }
                else {
                     // if not, maybe it was this other packet.
                    set_out_tcpconn(ptcpconn,srcip,dstip,srcport,dstport,TCP_STATE_UNKOWN,tot_len);
                    ptcpconn->finack_from_src_seq = ntohl(tcphdr->ack_seq) ;
                    printf("from server to client expect sequence=%x\n",ptcpconn->finack_from_src_seq);
                    return 0 ;
                }
            }
        }
    }
    else {
        if (ptcpconn->state == TCP_STATE_CLOSED){
            memset(ptcpconn,0,sizeof(tcpconnection));
            return 0 ;
        }
        //from client->server
        if(match(ptcpconn,srcip,dstip,srcport,dstport)){
            // if this is a fin going from cli->srv
            // expect an appropriate ack from server
            if(tcphdr->fin){
                ptcpconn->finack_from_dst_seq = ntohl(tcphdr->seq) + payload_len + 1;
            }
            switch(ptcpconn->state){
                case TCP_STATE_SYNACK_ACK:
                    if(tcphdr->ack)
                    //connection up
                      ptcpconn->state=TCP_STATE_UP;
                    break;
                case TCP_STATE_SYN_SYNACK:
                    if(tcphdr->syn && tcphdr->ack)
                    //SYN|ACK sent,awating ACK;
                      ptcpconn->state=TCP_STATE_SYNACK_ACK;
                    break;
                case TCP_STATE_UP:
                    if(tcphdr->fin)
                      //FIN sent,awating FIN|ACK
                      ptcpconn->state=TCP_STATE_FIN_FINACK;
                    break;
                case TCP_STATE_FIN_FINACK:
                    if(tcphdr->ack){
                        if(ntohl(tcphdr->ack_seq) == ptcpconn->finack_from_src_seq){
                            ptcpconn->isFinished |= RECV_FINACK_FROM_SRC;
                            if(ptcpconn->isFinished & RECV_FINACK_FROM_DST)
                                ptcpconn->state=TCP_STATE_CLOSED;
                        }
                    }
                    break;
                case TCP_STATE_UNKOWN:
                    printf("from client to server sequence=%x\n",ntohl(tcphdr->seq));
                    if(tcphdr->ack ){
                        if(ntohl(tcphdr->seq) == ptcpconn->finack_from_src_seq)
                          ptcpconn->state = TCP_STATE_UP;
                        else
                          ptcpconn->finack_from_dst_seq = ntohl(tcphdr->ack_seq) ;
                    }
                    break;
                default:
                    break;
            }
            if(tcphdr->rst){
                ptcpconn->state=TCP_STATE_RESET;
            }
            update_in_tcpconn(ptcpconn,payload_len<<3);
        }
        else if (match(ptcpconn,dstip,srcip,dstport,srcport)){
            if(tcphdr->fin){
                ptcpconn->finack_from_src_seq = ntohl(tcphdr->seq) + payload_len + 1;
            }
            switch(ptcpconn->state){
                case TCP_STATE_SYNACK_ACK:
                    if(tcphdr->ack)
                    //connection up
                      ptcpconn->state=TCP_STATE_UP;
                    break;
                case TCP_STATE_SYN_SYNACK:
                    if(tcphdr->syn && tcphdr->ack)
                    //SYN|ACK sent,awating ACK;
                      ptcpconn->state=TCP_STATE_SYNACK_ACK;
                    break;
                case TCP_STATE_UP:
                    if(tcphdr->fin)
                      //FIN sent,awating FIN|ACK
                      ptcpconn->state=TCP_STATE_FIN_FINACK;
                    break;
                case TCP_STATE_FIN_FINACK:
                    if(tcphdr->ack){
                        if(ntohl(tcphdr->ack_seq) == ptcpconn->finack_from_dst_seq){
                            ptcpconn->isFinished |= RECV_FINACK_FROM_DST;
                            if(ptcpconn->isFinished & RECV_FINACK_FROM_SRC)
                                ptcpconn->state=TCP_STATE_CLOSED;
                        }
                    }
                    break;
                case TCP_STATE_UNKOWN:
                    printf("from server to client sequence=%x\n",ntohl(tcphdr->seq));
                    if(tcphdr->ack ){
                        if(ntohl(tcphdr->seq) == ptcpconn->finack_from_dst_seq)
                          ptcpconn->state = TCP_STATE_UP;
                        else
                          ptcpconn->finack_from_src_seq = ntohl(tcphdr->ack_seq) ;
                    }
                    break;
                default:
                    break;
            }
            if(tcphdr->rst){
                ptcpconn->state=TCP_STATE_RESET;
            }
            update_out_tcpconn(ptcpconn,tot_len);
        }
            
			
	}
    return 0 ;
}
		
		
		
	


