#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include "tcpconnection.h"

extern void display_tcp_conn();
struct vlan_hdr{
    __u16 vlan_TCI;
    __u16 vlan_proto;
};
u_int16_t handle_ethernet ( __u16 *l2_len,const u_char* packet)
{
    struct ether_header *eptr;  /* net/ethernet.h */
    struct vlan_hdr *vlan;

    /* lets start with the ether header... */
    eptr = (struct ether_header *) packet;

#if 0
    fprintf(stdout,"ethernet header source: %s\n"
                ,ether_ntoa((const struct ether_addr *)&eptr->ether_shost));
    fprintf(stdout," destination: %s\n"
                ,ether_ntoa((const struct ether_addr *)&eptr->ether_dhost));
#endif 

    /* check to see if we have an ip packet */
    if (eptr->ether_type == htons(ETHERTYPE_IP))
    {
        *l2_len += 14;
    }else  if (eptr->ether_type == htons(ETHERTYPE_VLAN))
    {
        *l2_len += 4;
        vlan=(struct vlan_hdr *)(packet+14);
        return vlan->vlan_proto;
    }
    return eptr->ether_type;
}
/* callback function that is passed to pcap_loop(..) and called each time 
 * a packet is recieved                                                    */
void packet_callback(u_char *useless,const struct pcap_pkthdr* pkthdr,const u_char*
            packet)
{
    __u16 l2_len;
    __u16 type;
    _pkt  pkt;
    /* lets start with the ether header... */
    type=handle_ethernet(&l2_len,packet);
    if(type == htons(ETHERTYPE_IP))
    {/* handle IP packet */
    	//printf("l2_len=%d\n",l2_len);
        pkt.tot_len = pkthdr->len;
        pkt.l3_hdr  = packet+l2_len;
        ip_process(&pkt);
    }
    else 
      return ;
}

int main(int argc,char **argv)
{ 
    //char *dev; 
    int dlt;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* descr;

    if(argc != 3){ 
        fprintf(stdout,"Usage: %s -i <interface> \n",argv[0]);
        return 0;
    }
    if(strcmp(argv[1],"-i")){
        fprintf(stdout,"Usage: %s -i <interface> \n",argv[0]);
        return 0;
    }

#if 1
    if(init_tcpconn_table()!=0) 
        exit(1);
#endif
#if 0
    /* grab a device to peak into... */
    dev = pcap_lookupdev(argv[2]);
    if(dev == NULL){
        printf("%s\n",argv[2]);
        printf("R U root ?\n");
        exit(1); 
    }
#endif
    /* open the device for sniffing.

       pcap_t *pcap_open_live(char *device,int snaplen, int prmisc,int to_ms,
       char *ebuf)

       snaplen - maximum size of packets to capture in bytes
       promisc - set card in promiscuous mode?
       to_ms   - time to wait for packets in miliseconds before read
       times out
       errbuf  - if something happens, place error string here

       Note if you change "prmisc" param to anything other than zero, you will
       get all packets your device sees, whether they are intendeed for you or
       not!! Be sure you know the rules of the network you are running on
       before you set your card in promiscuous mode!!     */
    descr = pcap_open_live(argv[2],BUFSIZ,1,-1,errbuf);
    if(descr == NULL) {
        printf("pcap_open_live(): %s\n",errbuf);
        printf("R U root ?\n");
        exit(1);
    }
    dlt = pcap_datalink(descr); 
    if( dlt!=DLT_EN10MB  &&  dlt!=DLT_LINUX_SLL && dlt!=DLT_RAW && dlt!=DLT_NULL){
        printf("The specified interface type is not supported yet.\n");
        exit(1);
    }

    pthread_t thread;
    pthread_create(&thread,NULL,display_tcp_conn,NULL);
    /* allright here we call pcap_loop(..) and pass in our callback function */
    /* int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)*/
    /* If you are wondering what the user argument is all about, so am I!!   */
    pcap_loop(descr,0,packet_callback,NULL);

    return 0;
}

