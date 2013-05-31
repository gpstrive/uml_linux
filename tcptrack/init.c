/*-
 *   BSD LICENSE
 * 
 *   Copyright(c) 2010-2012 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *  version: DPDK.L.1.2.3-3
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/mman.h>
#include <sys/mman.h>
#include <errno.h>
#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_string_fns.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_lpm.h>

#include "main.h"
#include "stat.h"
#include "shm.h"
#include "port_contrl.h"
static struct rte_eth_conf port_conf = {
	.rxmode = {
		.split_hdr_size = 0,
		.header_split   = 0, /**< Header Split disabled */
		.hw_ip_checksum = 1, /**< IP checksum offload enabled */
		.hw_vlan_filter = 0, /**< VLAN filtering disabled */
		.jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
		.hw_strip_crc   = 0, /**< CRC stripped by hardware */
	},
	.rx_adv_conf = {
		.rss_conf = {
			.rss_key = NULL,
			.rss_hf = ETH_RSS_IPV4,
		},
	},
	.txmode = {
	},
};

static struct rte_eth_rxconf rx_conf = {
	.rx_thresh = {
		.pthresh = APP_DEFAULT_NIC_RX_PTHRESH,
		.hthresh = APP_DEFAULT_NIC_RX_HTHRESH,
		.wthresh = APP_DEFAULT_NIC_RX_WTHRESH,
	},
	.rx_free_thresh = APP_DEFAULT_NIC_RX_FREE_THRESH,
};

static struct rte_eth_txconf tx_conf = {
	.tx_thresh = {
		.pthresh = APP_DEFAULT_NIC_TX_PTHRESH,
		.hthresh = APP_DEFAULT_NIC_TX_HTHRESH,
		.wthresh = APP_DEFAULT_NIC_TX_WTHRESH,
	},
	.tx_free_thresh = APP_DEFAULT_NIC_TX_FREE_THRESH,
	.tx_rs_thresh = APP_DEFAULT_NIC_TX_RS_THRESH,
};
total_stat *ptotal_stat;

static uint32_t worker_id; 
// assume four worker_ids;
//#define WORK_NUM 4
//stat_info *pdst_stat_info[WORK_NUM];
stat_info *pdst_stat_info;
struct port_stat port_stat[MAX_PORT_NUM];
struct tcp_hashinfo_t tcp_hashinfo;

extern void app_init_comm();



static void
app_assign_worker_ids(void)
{
	uint32_t lcore;

	/* Assign ID for each worker */
	worker_id = 0;
	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		struct app_lcore_params_worker *lp_worker = &app.lcore_params[lcore].worker;

		if (app.lcore_params[lcore].type != e_APP_LCORE_WORKER) {
			continue;
		}

		lp_worker->worker_id = worker_id;
		worker_id ++;
	}
}

static void
app_init_mbuf_pools(void)
{
	uint32_t socket, lcore;

	/* Init the buffer pools */
	for (socket = 0; socket < APP_MAX_SOCKETS; socket ++) {
		char name[32];
		if (app_is_socket_used(socket) == 0) {
			continue;
		}

		rte_snprintf(name, sizeof(name), "mbuf_pool_%u", socket);
		printf("Creating the mbuf pool for socket %u ...\n", socket);
		app.pools[socket] = rte_mempool_create(
			name,
			APP_DEFAULT_MEMPOOL_BUFFERS,
			APP_DEFAULT_MBUF_SIZE,
			APP_DEFAULT_MEMPOOL_CACHE_SIZE,
			sizeof(struct rte_pktmbuf_pool_private),
			rte_pktmbuf_pool_init, NULL,
			rte_pktmbuf_init, NULL,
			socket,
			0);
		if (app.pools[socket] == NULL) {
			rte_panic("Cannot create mbuf pool on socket %u\n", socket);
		}
	}

	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		if (app.lcore_params[lcore].type == e_APP_LCORE_DISABLED) {
			continue;
		}

		socket = rte_lcore_to_socket_id(lcore);
		app.lcore_params[lcore].pool = app.pools[socket];
	}
}
/*
static void
app_init_lpm_tables(void)
{
	uint32_t socket, lcore;

	for (socket = 0; socket < APP_MAX_SOCKETS; socket ++) {
		char name[32];
		uint32_t rule;

		if (app_is_socket_used(socket) == 0) {
			continue;
		}

		rte_snprintf(name, sizeof(name), "lpm_table_%u", socket);
		printf("Creating the LPM table for socket %u ...\n", socket);
		app.lpm_tables[socket] = rte_lpm_create(
			name,
			socket,
			APP_MAX_LPM_RULES,
			RTE_LPM_MEMZONE);
		if (app.lpm_tables[socket] == NULL) {
			rte_panic("Unable to create LPM table on socket %u\n", socket);
		}

		for (rule = 0; rule < app.n_lpm_rules; rule ++) {
			int ret;

			ret = rte_lpm_add(app.lpm_tables[socket],
				app.lpm_rules[rule].ip,
				app.lpm_rules[rule].depth,
				app.lpm_rules[rule].if_out);

			if (ret < 0) {
				rte_panic("Unable to add entry %u (%x/%u => %u) to the LPM table on socket %u (%d)\n",
					rule, app.lpm_rules[rule].ip,
					(uint32_t) app.lpm_rules[rule].depth,
					(uint32_t) app.lpm_rules[rule].if_out,
					socket,
					ret);
			}
		}

	}

	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		if (app.lcore_params[lcore].type != e_APP_LCORE_WORKER) {
			continue;
		}

		socket = rte_lcore_to_socket_id(lcore);
		app.lcore_params[lcore].worker.lpm_table = app.lpm_tables[socket];
	}
}
*/
static void
app_init_rings_rx(void)
{
	uint32_t lcore;

	/* Initialize the rings for the RX side */
	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		struct app_lcore_params_io *lp_io = &app.lcore_params[lcore].io;
		uint32_t socket_io, lcore_worker;

		if ((app.lcore_params[lcore].type != e_APP_LCORE_IO) ||
		    (lp_io->rx.n_nic_queues == 0)) {
			continue;
		}

		socket_io = rte_lcore_to_socket_id(lcore);

		for (lcore_worker = 0; lcore_worker < APP_MAX_LCORES; lcore_worker ++) {
			char name[32];
			struct app_lcore_params_worker *lp_worker = &app.lcore_params[lcore_worker].worker;
			struct rte_ring *ring = NULL;

			if (app.lcore_params[lcore_worker].type != e_APP_LCORE_WORKER) {
				continue;
			}

			printf("Creating ring to connect I/O lcore %u (socket %u) with worker lcore %u ...\n",
				lcore,
				socket_io,
				lcore_worker);
			rte_snprintf(name, sizeof(name), "app_ring_rx_s%u_io%u_w%u",
				socket_io,
				lcore,
				lcore_worker);
			ring = rte_ring_create(
				name,
				app.ring_rx_size,
				socket_io,
				RING_F_SP_ENQ | RING_F_SC_DEQ);
			if (ring == NULL) {
				rte_panic("Cannot create ring to connect I/O core %u with worker core %u\n",
					lcore,
					lcore_worker);
			}

			lp_io->rx.rings[lp_io->rx.n_rings] = ring;
			lp_io->rx.n_rings ++;

			lp_worker->rings_in[lp_worker->n_rings_in] = ring;
			lp_worker->n_rings_in ++;
		}
	}

	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		struct app_lcore_params_io *lp_io = &app.lcore_params[lcore].io;

		if ((app.lcore_params[lcore].type != e_APP_LCORE_IO) ||
		    (lp_io->rx.n_nic_queues == 0)) {
			continue;
		}

		if (lp_io->rx.n_rings != app_get_lcores_worker()) {
			rte_panic("Algorithmic error (I/O RX rings)\n");
		}
	}

	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		struct app_lcore_params_worker *lp_worker = &app.lcore_params[lcore].worker;

		if (app.lcore_params[lcore].type != e_APP_LCORE_WORKER) {
			continue;
		}

		if (lp_worker->n_rings_in != app_get_lcores_io_rx()) {
			rte_panic("Algorithmic error (worker input rings)\n");
		}
	}
}


static void
app_init_rings_tx(void)
{
	uint32_t lcore;

	// Initialize the rings for the TX side 
	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		struct app_lcore_params_worker *lp_worker = &app.lcore_params[lcore].worker;
		uint32_t port;

		if (app.lcore_params[lcore].type != e_APP_LCORE_WORKER) {
			continue;
		}

		for (port = 0; port < APP_MAX_NIC_PORTS; port ++) {
			char name[32];
			struct app_lcore_params_io *lp_io = NULL;
			struct rte_ring *ring;
			uint32_t socket_io, lcore_io;

			if (app.nic_tx_port_mask[port] == 0) {
				continue;
			}

			if (app_get_lcore_for_nic_tx((uint8_t) port, &lcore_io) < 0) {
				rte_panic("Algorithmic error (no I/O core to handle TX of port %u)\n",
					port);
			}

			lp_io = &app.lcore_params[lcore_io].io;
			socket_io = rte_lcore_to_socket_id(lcore_io);

			printf("Creating ring to connect worker lcore %u with TX port %u (through I/O lcore %u) (socket %u) ...\n",
				lcore, port, lcore_io, socket_io);
			rte_snprintf(name, sizeof(name), "app_ring_tx_s%u_w%u_p%u", socket_io, lcore, port);
			ring = rte_ring_create(
				name,
				app.ring_tx_size,
				socket_io,
				RING_F_SP_ENQ | RING_F_SC_DEQ);
			if (ring == NULL) {
				rte_panic("Cannot create ring to connect worker core %u with TX port %u\n",
					lcore,
					port);
			}

			lp_worker->rings_out[port] = ring;
			lp_io->tx.rings[port][lp_worker->worker_id] = ring;
		}
	}

	for (lcore = 0; lcore < APP_MAX_LCORES; lcore ++) {
		struct app_lcore_params_io *lp_io = &app.lcore_params[lcore].io;
		uint32_t i;

		if ((app.lcore_params[lcore].type != e_APP_LCORE_IO) ||
		    (lp_io->tx.n_nic_ports == 0)) {
			continue;
		}

		for (i = 0; i < lp_io->tx.n_nic_ports; i ++){
			uint32_t port, j;

			port = lp_io->tx.nic_ports[i];
			for (j = 0; j < app_get_lcores_worker(); j ++) {
				if (lp_io->tx.rings[port][j] == NULL) {
					rte_panic("Algorithmic error (I/O TX rings)\n");
				}
			}
		}
	}
}

static void
app_init_nics(void)
{
	uint32_t socket, lcore;
	uint8_t	port, queue;
	struct ether_addr mac_addr;
	int ret;

	/* Init driver */
	printf("Initializing the PMD driver ...\n");
#ifdef RTE_LIBRTE_IGB_PMD
	if (rte_igb_pmd_init() < 0) {
		rte_panic("Cannot init IGB PMD\n");
	}
#endif
#ifdef RTE_LIBRTE_IXGBE_PMD
	if (rte_ixgbe_pmd_init() < 0) {
		rte_panic("Cannot init IXGBE PMD\n");
	}
#endif
	if (rte_eal_pci_probe() < 0) {
		rte_panic("Cannot probe PCI\n");
	}

	memset(port_stat,0,sizeof(struct port_stat)*MAX_PORT_NUM);
	/* Init NIC ports and queues, then start the ports */
	for (port = 0; port < APP_MAX_NIC_PORTS; port ++) {
		struct rte_eth_link link;
		struct rte_mempool *pool;
		uint32_t n_rx_queues, n_tx_queues;

		n_rx_queues = app_get_nic_rx_queues_per_port(port);
		n_tx_queues = app.nic_tx_port_mask[port];

		if ((n_rx_queues == 0) && (n_tx_queues == 0)) {
			continue;
		}

		/* Init port */
		printf("Initializing NIC port %u ...\n", (uint32_t) port);
		ret = rte_eth_dev_configure(
			port,
			(uint8_t) n_rx_queues,
			(uint8_t) n_tx_queues,
			&port_conf);
		if (ret < 0) {
			rte_panic("Cannot init NIC port %u (%d)\n", (uint32_t) port, ret);
		}
		rte_eth_promiscuous_enable(port);

		/* Init RX queues */
		for (queue = 0; queue < APP_MAX_RX_QUEUES_PER_NIC_PORT; queue ++) {
			if (app.nic_rx_queue_mask[port][queue] == 0) {
				continue;
			}

			app_get_lcore_for_nic_rx(port, queue, &lcore);
			socket = rte_lcore_to_socket_id(lcore);
			pool = app.lcore_params[lcore].pool;

			printf("Initializing NIC port %u RX queue %u ...\n",
				(uint32_t) port,
				(uint32_t) queue);
			ret = rte_eth_rx_queue_setup(
				port,
				queue,
				(uint16_t) app.nic_rx_ring_size,
				socket,
				&rx_conf,
				pool);
			if (ret < 0) {
				rte_panic("Cannot init RX queue %u for port %u (%d)\n",
					(uint32_t) queue,
					(uint32_t) port,
					ret);
			}
		}

		/* Init TX queues */
		
		if (app.nic_tx_port_mask[port] == 1) {
			app_get_lcore_for_nic_tx(port, &lcore);
			socket = rte_lcore_to_socket_id(lcore);
			printf("Initializing NIC port %u TX queue 0 ...\n",
				(uint32_t) port);
			ret = rte_eth_tx_queue_setup(
				port,
				0,
				(uint16_t) app.nic_tx_ring_size,
				socket,
				&tx_conf);
			if (ret < 0) {
				rte_panic("Cannot init TX queue 0 for port %d (%d)\n",
					port,
					ret);
			}
		}
		

		/* Start port */
		ret = rte_eth_dev_start(port);
		if (ret < 0) {
			rte_panic("Cannot start port %d (%d)\n", port, ret);
		}

		/* Get link status */
		rte_eth_link_get(port, &link);
		rte_eth_macaddr_get(port,&mac_addr);
		int i=0;
		for(i=0;i<5;i++)
			printf("%02x:",mac_addr.addr_bytes[i]);
		printf("%02x\n",mac_addr.addr_bytes[i]);
		memcpy(port_stat[port].mac_addr,mac_addr.addr_bytes,6);
		for(i=0;i<5;i++)
			printf("%02x:",port_stat[port].mac_addr[i]);
		printf("%02x\n",port_stat[port].mac_addr[i]);
		if (link.link_status) {
			printf("Port %u is UP (%u Mbps)\n",
				(uint32_t) port,
				(unsigned) link.link_speed);
			port_stat[port].port_status=1;
			port_stat[port].port_speed=link.link_speed;
		} else {
			printf("Port %u is DOWN\n",
				(uint32_t) port);
			port_stat[port].port_status=0;
		}
	}
}
#if 1
static int app_init_mmap(void)
{
	int fd;
	int result;
	
	fd=open(MMAP_TOTAL_FILE,O_CREAT|O_RDWR|O_TRUNC,0777);
	lseek(fd,sizeof(total_stat)-1,SEEK_SET);
	write(fd," ",1);
	//MAP_HUGETLB is  supported  since 2.6.32
	ptotal_stat=(total_stat *)mmap(NULL,sizeof(total_stat),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_LOCKED,fd,0);
	if(ptotal_stat == NULL){
		printf("mmap failed,Error num=%d\n",errno);
		return -1;
	}
	memset(ptotal_stat,0,sizeof(total_stat));
	close(fd);
	#if 0
	int i;
	if(unlikely(worker_id==0)){
		fprintf(stderr,"Warning,no worker lcore\n");
		return ;
	}
	if(worker_id > WORK_NUM){
		fprintf(stderr,"Please change pdst_stat_info definition\n");
		return ;
	}
	for (i=0;i<worker_id;i++){
		
		fd=open(MMAP_FILE(i),O_CREAT|O_RDWR|O_TRUNC,0777);
		lseek(fd,sizeof(stat_info)*STAT_INFO_SIZE-1,SEEK_SET);
		write(fd,"",1);
		PDST_STAT_INFO(i)=(stat_info*)mmap(NULL,sizeof(stat_info)*STAT_INFO_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_HUGETLB,fd,0);
		close(fd);
	}
	#endif
	fd=open(MMAP_FILE(0),O_CREAT|O_RDWR|O_TRUNC,0777);
	lseek(fd,sizeof(stat_info)*STAT_INFO_SIZE-1,SEEK_SET);
	write(fd," ",1);
	pdst_stat_info=(stat_info *)mmap(NULL,sizeof(stat_info)*STAT_INFO_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_LOCKED,fd,0);
	if(pdst_stat_info == NULL){
		printf("mmap failed,Error num=%d\n",errno);
		return -1;
	}
	else 
		printf("!!!!!!!!!!!!!mmap success,size=%x\n",sizeof(stat_info)*STAT_INFO_SIZE);
	memset(pdst_stat_info,0,sizeof(stat_info)*STAT_INFO_SIZE);
	close(fd);
	
	//init tcp half open state
	fd=open(MMAP_HALF_FILE,O_CREAT|O_RDWR|O_TRUNC,0777);
	result=lseek(fd,sizeof(tcp_listen)*LHASH_SIZE-1,SEEK_SET);
	if(result == -1){
		close(fd);
		perror("lhash lseek error\n");
		exit(-1);
	}
	//printf("lseek offset=%x\n",result);
	result=write(fd," ",1);
	if(result == -1){
		close(fd);
		perror("lhash write error\n");
		exit(-1);
	}
	//printf("written bytes=%x\n",result);
	tcp_hashinfo.lhash=(tcp_listen *)mmap(NULL,sizeof(tcp_listen)*LHASH_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_LOCKED,fd,0);
	if(tcp_hashinfo.lhash== NULL){
		printf("mmap failed,Error num=%d\n",errno);
		return -1;
	}
	else 
		printf("!!!!!!!!!!!!!mmap success,size=%x\n",sizeof(tcp_listen)*LHASH_SIZE);
	memset(tcp_hashinfo.lhash,0,sizeof(tcp_listen)*LHASH_SIZE);
	close(fd);

	//init tcp established state
	fd=open(MMAP_EST_FILE,O_CREAT|O_RDWR|O_TRUNC,0777);
	result=lseek(fd,sizeof(tcp_est)*EHASH_SIZE-1,SEEK_SET);
	if(result == -1){
		close(fd);
		perror("ehash lseek error\n");
		exit(-1);
	}
	//printf("lseek offset=%x\n",result);
	result=write(fd," ",1);
	if(result == -1){
		close(fd);
		perror("ehash write error\n");
		exit(-1);
	}
	//printf("written bytes=%x\n",result);
	tcp_hashinfo.ehash=(tcp_est *)mmap(NULL,sizeof(tcp_est)*EHASH_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_LOCKED,fd,0);
	if(tcp_hashinfo.ehash== NULL){
		printf("mmap failed,Error num=%d\n",errno);
		return -1;
	}
	else 
		printf("!!!!!!!!!!!!!mmap success,size=%x\n",sizeof(tcp_est)*EHASH_SIZE);
	memset(tcp_hashinfo.ehash,0,sizeof(tcp_est)*EHASH_SIZE);
	close(fd);
	
}
#endif
	
void
app_init(void)
{
	app_assign_worker_ids();
	app_init_mbuf_pools();
	//app_init_lpm_tables();
	app_init_rings_rx();
	app_init_rings_tx();
	app_init_nics();
	app_init_mmap();
	//app_init_comm();

	printf("Initialization completed.\n");
}
