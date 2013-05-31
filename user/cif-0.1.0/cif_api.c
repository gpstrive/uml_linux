#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/unistd.h>

//#ifdef __x86_64__
/* ARRGG! Broken System header... */
//#define gettid() syscall(__NR_gettid)
//#else
//_syscall0(pid_t, gettid)
//#endif

#include "cif.h"
#include "cif_intern.h"

#define CIF_IOCTL_BASE 0xCD

#define CIF_IOCTL_ADD_SIG   _IOW(CIF_IOCTL_BASE, 5, int)
#define CIF_IOCTL_DEL_SIG   _IOW(CIF_IOCTL_BASE, 6, int)
#define CIF_IOCTL_ACK_IRQ   _IOW(CIF_IOCTL_BASE, 7, int)

int cif_status_thread_stop;

/**
 * cif_status_thread - status thread
 */
void *cif_status_thread (void *ptr)
{
	uint8_t hostflags;
	char input_mem [0xe00]; /* input DPM size */
	struct cif *cif = (struct cif *)ptr;
	struct sched_param schedp;
	struct cif_msg msg_in;
	int policy;
	uint8_t *io, *io_plx;
	int busrunning=0;
	int prio = -1;
	int event_count;

	if (!cif)
		return (void *) -1;

	io = (uint8_t *)cif->io;
	io_plx = (uint8_t *)cif->io_plx;
	cif_status_thread_stop = 0;

	hostflags = io [0x1ffe];

	cif->prio = 84;
	prio = cif->prio;
	policy = cif->prio ? SCHED_FIFO : SCHED_OTHER;
	memset (&schedp, 0, sizeof (schedp));
	fflush(stderr);
	schedp.sched_priority = cif->prio;
	sched_setscheduler(0, policy, &schedp);

	memcpy (input_mem, io + 0xe00, 0xe00);

	while (!cif_status_thread_stop)	{
		/* Wait for interrupt */
		read(cif->fd, &event_count, sizeof(int));

		cif->devflags = io [0x1fff];
		cif->hostflags = io [0x1ffe];


		if ( !(cif->hostflags & 0x20) && !(cif->hostflags & 0x40)) {
			fflush(stderr);
			if (busrunning)
				busrunning = 0;

			else {
				fflush(stderr);
				usleep (500);
			}
		}
		else
			busrunning = 1;

		if ((io [0x1fff] & 1) != (cif->hostflags & 1)) {
			cif_msg_rx (cif, &msg_in, 0);
			/*cif_print_msg (&msg_in);*/

			/* Msg silently dropped */
		}

		if (cif->hostflags != hostflags) {
			hostflags = cif->hostflags;
			if (cif->pid  && ((io[0x1fff] & 4)
				!= (cif->hostflags & 4))) {
				/* hack to simulate change of state */

				if (memcmp(input_mem, io + 0xe00, 0xe00))
					memcpy (input_mem, io + 0xe00, 0xe00);

				usleep (800);

				if ((io[0x1fff] & 4) != (cif->hostflags & 4)) {
					if (io[0x1fff] & 4)
						io[0x1fff] &= ~4;
					else
						io[0x1fff] |= 4;
				}
			}
		}
		io_plx[0x4c] |= 0x01;
	} // while (!cif_status_thread_stop)...
	return NULL;
}

/**
 * cif_open - open a CIF-device
 */
struct cif *cif_open (const char *name, int prio)
{
	int size = 8192;
	struct cif *fi;
	pthread_mutexattr_t attr;

	cif_status_thread_stop = 0;

	fi = (struct cif *)calloc (1, sizeof (struct cif));
	if (!fi)
		goto out;

	fi->size = size;

	fi->fd = open (name, O_RDWR);
	if (fi->fd == -1)
		goto out_free;

	fi->io_plx = mmap (NULL, 128, PROT_READ | PROT_WRITE,
		       MAP_SHARED, fi->fd, 0 );
	if (fi->io_plx == MAP_FAILED)
		goto out_close;

	fi->io = mmap (NULL, size, PROT_READ | PROT_WRITE,
		       MAP_SHARED, fi->fd, getpagesize() );
	if (fi->io == MAP_FAILED)
		goto out_close;

	fi->prio = prio;

	if (pthread_create (&fi->status_thread, NULL, cif_status_thread, fi))
		goto out_close;

	pthread_mutexattr_init (&attr);
#ifdef HAVE_PTHREAD_MUTEXATTR_SETPROTOCOL
	pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
#ifdef HAVE_PTHREAD_MUTEXATTR_SETROBUST_NP
	pthread_mutexattr_setrobust_np(&attr, PTHREAD_MUTEX_ROBUST_NP);
#endif
#endif
	pthread_mutex_init (&fi->msg_lock, &attr);

	return fi;
out_close:
	close (fi->fd);
out_free:
	free (fi);
out:
	return NULL;
}

/**
 * cif_set_fieldbus -
 */

int cif_set_fieldbus (struct cif *fi, enum civ_type type)
{
	if (!fi)
		return -1;

	fi->type = type;

	return 0;
}

/**
 * cif_close - close a CIF-device
 */
int cif_close (struct cif *fi)
{
	if (!fi)
		return -1;
	munmap (fi->io, fi->size);
	close (fi->fd);

	pthread_mutex_destroy (&fi->msg_lock);

	pthread_kill (fi->status_thread, SIGTERM);

	free (fi);

	return 0;
}

/**
 * cif_list - list CIF-devices
 */
char **cif_list (void)
{
	return NULL;
}

static inline void tsnorm(struct timespec *ts)
{
	while (ts->tv_nsec >= 1000000000) {
		ts->tv_nsec -= 1000000000;
		ts->tv_sec++;
	}
}

/**
 * cif_msg_tx - transmit a message
 */

int cif_msg_tx (struct cif *fi, struct cif_msg *buf,
		struct timespec *timeout_send,  struct timespec *timeout_ack)
{
	uint8_t *io;
	struct timespec now;
	int cntr = 0;

	if (!fi || !buf)
		return -1;

	io = (uint8_t *)cif_get_io (fi);

	/* clear to send? */
	/* Warten ob die Nachricht gesendet werden kann */
	while ( (fi->hostflags &2) != (io [0x1fff] &2)  )
	{
		clock_gettime (CLOCK_MONOTONIC, &now);
		cntr++;
		if (  (now.tv_sec >= timeout_send->tv_sec)
		    &&(now.tv_nsec >= timeout_send->tv_nsec) ) {
			/*fprintf(stderr, " now %d:%d, timeout %d:%d cntr%d\n",
			  now.tv_sec, now.tv_nsec,
			  timeout_send->tv_sec,timeout_send->tv_nsec, cntr);
			  fflush(stderr);*/
			return -3;
		}
		usleep (200);
	}

	pthread_mutex_lock(&fi->msg_lock);
	memcpy (io + 0x1c00, &buf->msg, sizeof (struct cif_msg));
	if ( io [0x1fff] & 2)
		io [0x1fff] &= ~2;
	else
		io [0x1fff] |= 2;
	pthread_mutex_unlock(&fi->msg_lock);

	return 0;
}

/**
 * cif_msg_rx - receive a message
 */

int cif_msg_rx (struct cif *fi, struct cif_msg *buf, struct timespec *timeout)
{
	uint8_t *io;
	struct timespec now;

	if (!fi || !buf)
		return -1;

	io = (uint8_t *)cif_get_io (fi);

	while ( (fi->hostflags &1) == (io [0x1fff] &1)  )
	{
		clock_gettime (CLOCK_MONOTONIC, &now);
		if ((now.tv_sec >= timeout->tv_sec) &&
		    (now.tv_nsec >= timeout->tv_nsec)) {
			return -3;
		}
		usleep (100);
	}

	pthread_mutex_lock(&fi->msg_lock);
	memcpy (&buf->msg, io + 0x1d40, sizeof (struct cif_msg));
	if ( io [0x1fff] & 1)
		io [0x1fff] &= ~1;
	else
		io [0x1fff] |= 1;

	pthread_mutex_unlock(&fi->msg_lock);

	return 0;
}

/**
 * cif_get_io - get pointer to mmaped IO
 */
volatile void *cif_get_io (struct cif *fi)
{
	if (!fi)
		return NULL;

	return fi->io;
}

/**
 * cif_get_io_plx - get pointer to mmaped PLX-IO
 */
volatile void *cif_get_io_plx (struct cif *fi)
{
	if (!fi)
		return NULL;

	return fi->io_plx;
}

/*
void cif_fill_msg_header (struct cif_msg *msg, uint8_t rx,
					uint8_t tx, uint8_t ln, uint8_t nr,
					uint8_t a, uint8_t f, uint8_t b,
					uint8_t e)
*/

int cif_conf_upload_mem (struct cif *cif, uint8_t *conf,  unsigned int len)
{
	unsigned int idx = 52;
	uint8_t *io, nr = 1;
	uint32_t offset;
	struct cif_msg msg;

	if (!cif || !conf || !len)
		return -1;

	cif_dev_boot (cif);
	sleep (10);

	io = (uint8_t *)cif_get_io (cif);

	cif_fill_msg_header (&msg, 0, 0xff, 51, nr++, 0, 0, 6, RCS_FIRST_MSK);

	/* Hilscher magic */
	offset = conf [40] + (conf [41]<<8) +
		(conf [42]<<16) + (conf [43]<<24);
	offset += 2*(conf [60] + (conf [61]<<8));
	offset += 14;

	offset = 2*(conf [offset] + (conf [offset + 1]<<8));
	offset += conf [40] + (conf [41]<<8) + (conf [42]<<16) + (conf [43]<<24);
	offset += 11;

	msg.msg.tel_head.dev_addr = 3;
	memcpy (&msg.msg.tel_head.data_area, conf + 44, 16);
	memcpy (msg.msg.d + 8, conf + offset, 35);

	while (!cif_dev_mbx_empty (cif))
		usleep (100);

	while (!cif_host_mbx_full (cif))
		usleep (10000);

	while (len > idx + 240)
	{
		cif_fill_msg_header (&msg, 0, 0xff, 240, nr++, 0, 0, 6,
				     RCS_CONT_MSK);
		memcpy (&msg.msg.tel_head, conf + idx, 240);
		idx += 240;

		while (!cif_dev_mbx_empty (cif))
			usleep (100);

		while (!cif_host_mbx_full (cif))
			usleep (10000);
	}

	cif_fill_msg_header (&msg, 0, 0xff, len - idx, nr++, 0, 0, 6,
			     RCS_LAST_MSK);
	memcpy (&msg.msg.tel_head, conf + idx, len - idx);

	while (!cif_dev_mbx_empty (cif))
		usleep (100);

	return 0;
}

/**
 * cif_conf_upload - upload a configuration
 */

int cif_conf_upload (struct cif *cif, const char *filename)
{
	uint8_t *conf;
	unsigned int len;
	int fd, ret = -1;

	if (!cif || !filename)
		return ret;

	fd = open (filename, O_RDONLY);
	if (fd == -1)
		return -2;

	conf = (uint8_t *)malloc (8192);
	if (!conf)
		goto out;

	len = read (fd, conf, 8192);

	ret = cif_conf_upload_mem (cif, conf, len);
	free (conf);
out:
	close (fd);

	return ret;
}

/**
 * print_asc - print buffer as ASCII char
 */

static void print_asc (uint8_t *buf, int len)
{
	unsigned char ch;
	int i;

	for (i = 0; i < len; i++)
	{
		ch = buf [i];
		printf ("%c", (isprint(ch)) ? ch : '.');
	}
	printf ("\n");
}

/**
 * cif_print_info - print general information
 */

int cif_print_info (struct cif *fi)
{
	uint8_t *io;

	if (!fi)
		return -1;

	io = (uint8_t *)cif_get_io (fi);

	printf ("CIF info:\n");
	printf ("Date          = %02x.%02x.%02x%02x\n",
		io [0x1d20], io [0x1d21], io [0x1d22], io [0x1d23]);
	printf ("Device Number = 0x%08lx\n",
		*((unsigned long *)(io + 0x1d24)));
	printf ("Serial Number = 0x%08lx\n",
		*((unsigned long *)(io + 0x1d28)));

	printf ("PC OS Name    = ");
	print_asc (io + 0x1d30, 12);

	printf ("OEM Id        = ");
	print_asc (io + 0x1d3c, 4);

	printf ("Firmware Name = ");
	print_asc (io + 0x1e60, 16);

	printf ("Firmware Ver. = ");
	print_asc (io + 0x1e70, 16);

	printf ("RCS Version   = %d\n",
		*((unsigned short *) (io + 0x1ff0)));
	printf ("RCS Error     = ");
	switch (io [0x1ff2])
	{
	case 255:
		printf ("Initialisation after reset\n");
		break;
	case 254:
		printf ("Initialisation after watchdog failed\n");
		break;
	case 253:
		printf ("Bootstraploader active\n\n");
		break;
	case 252:
		printf ("Download active\n\n");
		break;
	case 0:
		printf ("firmware is running\n");
		break;
	default:
		printf ("unknown error %d\n", io [0x1ff2]);
	}
	printf ("HostWatchDog  = %d\n", io [0x1ff3]);
	printf ("DevWatchDog   = %d\n", io [0x1ff4]);
	printf ("Segment Count = %d\n", io [0x1ff5]);
	printf ("Device Addr.  = %d\n", io [0x1ff6]);
	printf ("Driver Type   = %d\n", io [0x1ff7]);
	printf ("DPM Size      = %d kiB\n", io [0x1ff8]);
	printf ("Device Type   = %d\n", io [0x1ff9]);
	printf ("Device Model  = %d\n", io [0x1ffa]);
	printf ("OEM Id        = %c%c%c\n",
		(isprint(io [0x1ffb])) ? io [0x1ffb] : '.',
		(isprint(io [0x1ffc])) ? io [0x1ffc] : '.',
		(isprint(io [0x1ffd])) ? io [0x1ffd]: '.');

	printf ("\n");

	return 0;
}

/**
 * cif_print_taskinfo print task information
 */

int cif_print_taskinfo (struct cif *fi, int task)
{
	uint8_t *io;
	unsigned char cond;

	if (!fi || (task < 1) || (task > 7))
		return -1;

	io = (uint8_t *)cif_get_io (fi);

	printf ("Task%d Name    = ", task);
	print_asc (io + 0x1f80 + (task -1)*0x10, 8);

	printf ("Task%d Version = %d\n", task, io [0x1f88 + (task - 1)*0x10]);
	printf ("Task%d Cond.   = ", task);

	cond = io [0x1f8a + (task - 1)*0x10];
	switch (cond)
	{
	case 255:
		printf ("not present\n");
		break;

	case 254:
		printf ("configuration\n");
		break;

	case 253:
		printf ("included\n");
		break;

	case 252:
		printf ("activated\n");
		break;

	case 251:
		printf ("ready for configuration\n");
		break;

	case 250:
		printf ("initialized\n");
		break;

	case 2:
		printf ("don't transfer process data\n");
		break;

	case 1:
		printf ("blocked\n");
		break;

	case 0:
		printf ("running\n");
		break;

	default:
		if ((cond > 239) || (cond < 10))
			printf ("reseved\n");
		else
			printf ("init error %d\n", cond);
	}

	if (task == 1 || task == 2)
	{
		printf ("Task%d Param   = ", task);
		print_asc (io + 0x1e80 + (task - 1)*0x40, 64);
		printf ("Task%d State   = ", task);
		print_asc (io + 0x1f00 + (task - 1)*0x40, 64);
	}
	printf ("\n");
	return 0;
}

/**
 * cif_print_state - print state information
 */

int cif_print_state (struct cif *fi)
{
	uint8_t *io;

	if (!fi)
		return -1;
	io = (uint8_t *)cif_get_io (fi);

	/* don't do that at home!
	 * The interrupt gates are opened...
	 */
	printf ("CIF state:\n");
	printf ("HostFlags = 0x%02x\n", fi->hostflags);
	printf ("DevFlags  = 0x%02x\n", io [0x1fff]);
	printf ("\n");

	return 0;
}

#define DUALPORT_INPUT_LEN 3584
#define DUALPORT_OUTPUT_LEN 3584

/**
 * cif_pd_out_read - read Process data (output)
 */
int cif_pd_out_read (struct cif *fi, unsigned int num, uint8_t *buf)
{
	uint8_t *io;

	if (!fi || !buf || (num == 0) || (num > DUALPORT_OUTPUT_LEN))
		return -1;

	io = (uint8_t *)cif_get_io (fi);

	memcpy (buf, io + 0xe00, num);

	fprintf (stderr, "%s (%d): d:%02x h:%02x\n", __FUNCTION__, getpid (),
		 io [0x1fff], fi->hostflags);
	fflush (stderr);

	return 0;
}

/**
 * cif_pd_in_write - write Process data (input)
 */
int cif_pd_in_write (struct cif *fi, unsigned int num, uint8_t *buf)
{
	uint8_t *io;

	if (!fi || !buf || (num == 0) || (num > DUALPORT_OUTPUT_LEN))
		return -1;

	io = (uint8_t *)cif_get_io (fi);

	memcpy (io, buf, num);
	if ((io [0x1fff] & 8) != (fi->hostflags & 8)) {
		if (io [0x1fff] & 8)
			io [0x1fff] &= ~8; // tilde 8
		else
			io [0x1fff] |= 8;
	}
	return 0;
}

#define CIF_MAX_WRITE_LEN 244

/**
 * cif_pd_in_write_msg - write Process data (input, Msg. based)
 */
int cif_pd_in_write_msg (struct cif *fi, unsigned int num, uint8_t *buf,
			 struct timespec *timeout_send,
			 struct timespec *timeout_ack)
{
	static int nr = 0;
	uint8_t *io;
	struct cif_msg out;

	if (!fi || !buf || (num == 0) || (num > CIF_MAX_WRITE_LEN))
		return -1;

	io = (uint8_t *)cif_get_io (fi);

	out.res = 0;
	out.msg.msg_head.rx = 2;
	out.msg.msg_head.tx = 16;
	out.msg.msg_head.ln = 8 + num;
	out.msg.msg_head.nr = (nr++)%256;
	out.msg.msg_head.a = 0;
	out.msg.msg_head.f = 0;
	out.msg.msg_head.b = 212;
	out.msg.msg_head.e = 0;

	out.msg.tel_head.dev_addr = 0;
	out.msg.tel_head.data_area = 1;
	out.msg.tel_head.data_addr = 0;
	out.msg.tel_head.data_idx = 0;
	out.msg.tel_head.data_cnt = num;
	out.msg.tel_head.data_type = 5;
	out.msg.tel_head.fnc = 2;
	memcpy (out.msg.d, buf, num);

	return cif_msg_tx (fi, &out, timeout_send, timeout_ack);
}

/**
 * cif_dev_init - (re)initialize a device
 */

int cif_dev_init (struct cif *fi)
{
	uint8_t *io;
	unsigned long timeout = 0;

	if (!fi)
		return -1;
	io = (uint8_t *)cif_get_io (fi);

	io [0x1fff] = CIF_DEV_INIT;

	//while (!(fi->hostflags & 0x80)) {
	while (!(io[0x1ffe] & 0x80)) {
		usleep (100);
		timeout++;
		if (timeout>1000) {
			fprintf(stderr,"Timeout in cif_dev_init().\n");
			return -1;
		}
	}

	return 0;
}

/**
 * cif_dev_reset - reset a device (warmstart)
 */

int cif_dev_reset (struct cif *fi)
{
	uint8_t *io;
	unsigned long timeout = 0;

	if (!fi)
		return -1;
	io = (uint8_t *)cif_get_io (fi);

	io [0x1fff] = CIF_DEV_RESET;

	while (!(fi->hostflags & 0x80)) {
		usleep (100);
		timeout++;
		if (timeout>1000) {
			fprintf(stderr,"Timeout in cif_dev_reset().\n");
			return -1;
		}
	}

	return 0;
}

/**
 * cif_dev_boot - boot a device (coldstart)
 */

int cif_dev_boot (struct cif *fi)
{
	uint8_t *io;
	unsigned long timeout = 0;

	if (!fi)
		return -1;
	io = (uint8_t *)cif_get_io (fi);

	io [0x1fff] = CIF_DEV_BOOT;

	while (!(fi->hostflags & 0x80)) {
		usleep (100);
		timeout++;
		if (timeout>1000) {
			fprintf(stderr,"Timeout in cif_dev_boot().\n");
			return -1;
		}
	}

	return 0;
}

/**
 * cif_dev_mbx_empty - device mailbox empty?
 */

int cif_dev_mbx_empty (struct cif *fi)
{
	uint8_t *io;

	if (!fi)
		return -1;
	io = (uint8_t *)cif_get_io (fi);

	if (((fi->hostflags ^ io [0x1fff]) & 2) == 0)
		return 1;
	else
		return 0;
}

/**
 * cif_host_mbx_full - host mailbox full?
 */

int cif_host_mbx_full (struct cif *fi)
{
	uint8_t *io;

	if (!fi)
		return -1;
	io = (uint8_t *)cif_get_io (fi);

	if (((fi->hostflags ^ io [0x1fff]) & 1) == 0)
		return 0;
	else
		return 1;
}

/**
 * cif_print_msg - print a CIF Message
 */

void cif_print_msg (struct cif_msg *msg)
{
	int i;

	printf ("CIF Message:\n");
	printf ("Message Header:\n");
	printf ("rx = 0x%02x\n", msg->msg.msg_head.rx);
	printf ("tx = 0x%02x\n", msg->msg.msg_head.tx);
	printf ("ln = 0x%02x\n", msg->msg.msg_head.ln);
	printf ("nr = 0x%02x\n", msg->msg.msg_head.nr);
	printf ("a  = 0x%02x\n", msg->msg.msg_head.a);
	printf ("f  = 0x%02x\n", msg->msg.msg_head.f);
	printf ("b  = 0x%02x\n", msg->msg.msg_head.b);
	printf ("e  = 0x%02x\n", msg->msg.msg_head.e);
	printf ("\n");

	printf ("Telegramm Header:\n");
	printf ("dev_addr  = 0x%02x\n", msg->msg.tel_head.dev_addr);
	printf ("data_area = 0x%02x\n", msg->msg.tel_head.data_area);
	printf ("data_addr = 0x%04x\n", msg->msg.tel_head.data_addr);
	printf ("data_idx  = 0x%02x\n", msg->msg.tel_head.data_idx);
	printf ("data_cnt  = 0x%02x\n", msg->msg.tel_head.data_cnt);
	printf ("data_type = 0x%02x\n", msg->msg.tel_head.data_type);
	printf ("function  = 0x%02x\n", msg->msg.tel_head.fnc);
	printf ("\n");

	printf ("Data:\n");
	for (i = 0; i < msg->msg.msg_head.ln - 8; i++)
		printf ("%02x%c", msg->msg.d [i], ((i + 1)%16) ? ' ' : '\n');
	printf ("\n");

	printf ("\n");
	fflush (stdout);
}

/**
 * cif_print_msg - print a CIF Message
 */

void cif_print_msg_dump (struct cif_msg *msg)
{
	int i;
	unsigned char *ch = (unsigned char *) &msg->msg;

	printf ("Msg. Dump:\n");
	for (i = 0; i < msg->msg.msg_head.ln + 8; i++)
		printf ("%02x%c", ch [i], ((i + 1)%16) ? ' ' : '\n');
	printf ("\n");

	printf ("\n");
	fflush (stdout);
}
