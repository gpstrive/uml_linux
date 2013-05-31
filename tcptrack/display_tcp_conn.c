#include <curses.h>
#include "tcpconnection.h"
#define TCP_CONN_FILE "/dev/shm/tcp_conn.log"
extern tcpconnection *tcpconn_table;
#ifndef unlikely
#define unlikely(x) __builtin_expect((x),0)
#endif
u32 getidletime(u16 lasttime){
    u32 idletime;
    u16 now=time(0);
    if(now < lasttime)
        idletime=(1<<16)+now-lasttime;
    else
        idletime=now-lasttime;
    return idletime;
}
#ifdef EXPORT_TO_FILE
void printidletime(FILE *file,u32 idletime){
    if(idletime > 3600)
      fprintf(file,"%dh  ",idletime/3600);
    else if (idletime > 60)
      fprintf(file,"%dm  ",idletime/60);
    else
      fprintf(file,"%ds  ",idletime);
}
void printpps(FILE *file,u32 pps){
    char tmp[9];
    printf("pps: %u\n",pps);
    if(pps > 1000000){

        sprintf(tmp,"%dMpps",pps/1000000);
        fprintf(file,"%-9s",tmp);
    }
    else if( pps > 1000){
        sprintf(tmp,"%dKpps",pps/1000);
        fprintf(file,"%-9s",tmp);
    }
    else{
        sprintf(tmp,"%dpps",pps);
        fprintf(file,"%-9s",tmp);
    }
}
void printBps(FILE *file,u32 Bps){
    char tmp[9];
    printf("Bps: %u\n",Bps);
    if(Bps > 1000000){

        sprintf(tmp,"%dMBps",Bps/1000000);
        fprintf(file,"%-9s",tmp);
    }
    else if( Bps > 1000){
        sprintf(tmp,"%dKBps",Bps/1000);
        fprintf(file,"%-9s",tmp);
    }
    else{
        sprintf(tmp,"%dBps",Bps);
        fprintf(file,"%-9s",tmp);
    }
}
#else
void printidletime(u32 idletime){
    if(idletime > 3600)
      printf("%dh",idletime/3600);
    else if (idletime > 60)
      printf("%dm",idletime/60);
    else
      printf("%ds",idletime);
}
void printpps(u32 pps){
    if(pps > 1000000)
        printf("%dMpps",pps/1000000);
    else if( pps > 1000)
        printf("%dKpps",pps/1000);
    else
        printf("%dpps",pps);
}
#endif
#define IPPORT_LEN 25
void display_tcp_conn(){
    u32 i,idletime;
    u8 tcp_state;
    tcpconnection *ptcpconn;
    char ipport[IPPORT_LEN];
	FILE *file;
	
	while(true){
#ifdef EXPORT_TO_FILE
		file = fopen(TCP_CONN_FILE,"w");
		if(unlikely(file == NULL)){
			printf("Error,open %s\n",TCP_CONN_FILE);
			exit(-1);
		}
	    fprintf(file,"%-22s","Client");
	    fprintf(file,"%-22s","Server");
	    fprintf(file,"%-13s","State");
	    fprintf(file,"%-9s","inSpeed");
	    fprintf(file,"%-9s","outSpeed ");
	    fprintf(file,"Idle\n");
	    for(i=0;i<TCPCONN_TABLE_SIZE;i++){
	        ptcpconn=&tcpconn_table[i];
	        if(ptcpconn->dstip){
                idletime=getidletime(ptcpconn->last_pkt_timestamp);
                tcp_state = ptcpconn->state;
                memset(ipport,0,IPPORT_LEN);
	            sprintf(ipport,"%d.%d.%d.%d:%d  ",NIPQUAD(ptcpconn->srcip),ptcpconn->srcport);
	            fprintf(file,"%-22s",ipport);
                memset(ipport,0,IPPORT_LEN);
	            sprintf(ipport,"%d.%d.%d.%d:%d  ",NIPQUAD(ptcpconn->dstip),ptcpconn->dstport);
	            fprintf(file,"%-22s",ipport);
	            switch(tcp_state){
	                case TCP_STATE_SYN_SYNACK:
	                  fprintf(file,"%-13s","SYN_SENT");
	                  break;
	                case TCP_STATE_SYNACK_ACK:
	                  fprintf(file,"%-13s","SYN|ACK-ACK");
	                  break;
	                case TCP_STATE_UP :
	                  fprintf(file,"%-13s","ESTABLISHED");
	                  break;
	                case TCP_STATE_FIN_FINACK:
	                  fprintf(file,"%-13s","CLOSING");
	                  break;
	                case TCP_STATE_CLOSED:
	                  fprintf(file,"%-13s","CLOSED ");
	                  break;
	                case TCP_STATE_RESET:
	                  fprintf(file,"%-13s","RESET");
	                  break;
	                default :
	                  break;
	            }
	            printBps(file,ptcpconn->inBps);
	            printBps(file,ptcpconn->outBps);
                memset(&ptcpconn->inBps,0,sizeof(u32)*4);//inBps,inpps,outBps,outpps;
	            printidletime(file,idletime);
				fprintf(file,"\n");
                if(idletime > tcp_timeouts[tcp_state]){
                    memset(ptcpconn,0,sizeof(tcpconnection));
                }
	        }
    	}
		fclose(file);
#else
        int row;
        erase();
        attron(A_REVERSE);
        move(0,0);
        printf("                                                                               ");
        move(0,1);
        printf("Client");
        move(0,23);
        printf("Server");
        move(0,45);
        printf("State");
        move(0,58);
        printf("inSpeed");
        move(0,65);
        printf("outSpeed");
        move(0,72);
        printf("Idle");

        attroff(A_REVERSE);



        move(1,0);

	    for(i=0;i<TCPCONN_TABLE_SIZE;i++){
	        ptcpconn=&tcpconn_table[i];
	        if(ptcpconn->dstip){
                move(row,1);
                idletime=getidletime(ptcpconn->last_pkt_timestamp);
                tcp_state = ptcpconn->state;
	            printf("%d.%d.%d.%d:%d",NIPQUAD(ptcpconn->srcip),ptcpconn->srcport);
                move(row,23);
	            printf("%d.%d.%d.%d:%d",NIPQUAD(ptcpconn->dstip),ptcpconn->dstport);
                move(row,45);
	            switch(tcp_state){
	                case TCP_STATE_SYN_SYNACK:

	                  printf("SYN_SENT");
	                  break;
	                case TCP_STATE_SYNACK_ACK:
	                  printf("SYN|ACK-ACK");
	                  break;
	                case TCP_STATE_UP :
	                  printf("ESTABLISHED");
	                  break;
	                case TCP_STATE_FIN_FINACK:
	                  printf("CLOSING");
	                  break;
	                case TCP_STATE_CLOSED:
	                  printf("CLOSED");
	                  break;
	                case TCP_STATE_RESET:
	                  printf("RESET");
	                  break;
	                default :
	                  break;
	            }
                move(row,58);
	            printpps(ptcpconn->inpps);
                move(row,65);
	            printpps(ptcpconn->outpps);
                move(row,72);
                memset(&ptcpconn->inbps,0,sizeof(u32)*4);//inbps,inpps,outbps,outpps;
	            printidletime(idletime);
                if(idletime > tcp_timeouts[tcp_state]){
                    memset(ptcpconn,0,sizeof(tcpconnection));
                }
                row++;
	        }
    	}
        row=0;
#endif
		sleep(1);
	}
}



    

