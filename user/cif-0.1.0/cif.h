/* cif.h */

#ifndef CIF_H
#define CIF_H
#include <pthread.h>
#include <signal.h>

struct msg_header {
	uint8_t	rx;
	uint8_t	tx;
	uint8_t	ln;
	uint8_t	nr;
	uint8_t	a;
	uint8_t	f;
	uint8_t	b;
	uint8_t	e;
} __attribute__((packed));

struct tel_header {
	uint8_t	dev_addr;
	uint8_t	data_area;
	uint16_t data_addr;
	uint8_t	data_idx;
	uint8_t	data_cnt;
	uint8_t	data_type;
	uint8_t	fnc;
} __attribute__((packed));

struct msg {
	struct msg_header	msg_head;
	struct tel_header	tel_head;
	uint8_t			d[247];
	uint8_t			dummy[25];
} __attribute__((packed));

struct cif_msg
{
	unsigned short	res; /* should be 0 */
	struct msg	msg;
	unsigned long	timeout;
	short		err;
} __attribute__((packed));

enum civ_type {
	CIV_DPS,
	CIV_DPM,
	CIV_COM,
};

struct cif
{
	int fd;
	int prio;
	size_t size;
	uint8_t devflags;
	uint8_t hostflags;
	int pid;
	int sig;
	int msg_req;
	union sigval sigprocessdata;
	union sigval sigerror;
	union sigval sigmsg;
	volatile void *io;
	volatile void *io_plx;
	enum civ_type type;
	pthread_t status_thread;
	pthread_mutex_t msg_lock;
};

#define CIF_DEV_INIT	0x40
#define CIF_DEV_RESET	0x80
#define CIF_DEV_BOOT	0xC0

/* helper */
static inline void cif_fill_msg_header (struct cif_msg *msg, uint8_t rx,
					uint8_t tx, uint8_t ln, uint8_t nr,
					uint8_t a, uint8_t f, uint8_t b,
					uint8_t e)
{
	msg->msg.msg_head.rx = rx;
	msg->msg.msg_head.tx = tx;
	msg->msg.msg_head.ln = ln;
	msg->msg.msg_head.nr = nr;
	msg->msg.msg_head.a  = a;
	msg->msg.msg_head.f  = f;
	msg->msg.msg_head.b  = b;
	msg->msg.msg_head.e  = e;
}

static inline void cif_fill_tel_header (struct cif_msg *msg, uint8_t dev,
					uint8_t area, uint16_t addr,
					uint8_t idx, uint8_t cnt,
					uint8_t type, uint8_t fnc)
{
	msg->msg.tel_head.dev_addr = dev;
	msg->msg.tel_head.data_area = area;
	msg->msg.tel_head.data_addr = addr;
	msg->msg.tel_head.data_idx = idx;
	msg->msg.tel_head.data_cnt = cnt;
	msg->msg.tel_head.data_type = type;
	msg->msg.tel_head.fnc = fnc;
}

/* prototypes */
struct cif *cif_open (const char *name, int prio);
int cif_close (struct cif *fi);
char **cif_list (void);
int cif_msg_rx (struct cif *fi, struct cif_msg *buf, struct timespec *timeout);
int cif_msg_tx (struct cif *fi, struct cif_msg *buf,
		struct timespec *timeout_send,  struct timespec *timeout_ack);
volatile void *cif_get_io (struct cif *fi);
volatile void *cif_get_io_plx (struct cif *fi);
int cif_print_info (struct cif *fi);
int cif_print_state (struct cif *fi);
int cif_dev_init (struct cif *fi);
int cif_dev_reset (struct cif *fi);
int cif_dev_boot (struct cif *fi);
void cif_print_msg (struct cif_msg *msg);
int cif_conf_upload_mem (struct cif *cif, uint8_t *conf, unsigned int len);
int cif_conf_upload (struct cif *cif, const char *filename);
int cif_dev_mbx_empty (struct cif *fi);
int cif_host_mbx_full (struct cif *fi);
int cif_pd_out_read (struct cif *fi, unsigned int num, uint8_t *buf);
int cif_pd_in_write (struct cif *fi, unsigned int num, uint8_t *buf);
int cif_pd_in_write_msg (struct cif *fi, unsigned int num, uint8_t *buf,
			 struct timespec *timeout_send,
			 struct timespec *timeout_ack);


#define RCS_FIFO_MSK                           0
#define RCS_LIFO_MSK                           1
#define RCS_NAK_MSK                            2

#define RCS_NORM_MSK                           0
#define RCS_FIRST_MSK                          4
#define RCS_CONT_MSK                           8
#define RCS_LAST_MSK                         0x0C
#define RCS_SEQ_MSK                          0x0C

extern uint8_t cif_devflags, cif_hostflags;
#endif
