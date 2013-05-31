#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "cif.h"
#include "cif_dps.h"

/**
 * cif_dps_info - show DP slave parameter and status 
 */

int cif_dps_info (struct cif *cif)
{
	uint8_t *io;
	int i;
	
	if (!cif)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
        
	printf ("Profibus DP slave:\n");
        printf ("Mode:\n");
        printf ("bPlcMode         = 0x%02x\n", io [0x1ec0]);
        printf ("usWatchdogTime   = 0x%d\n", 
		*((unsigned short *)(io + 0x1ec1)));
	printf ("\n");
	
        printf ("Setup:\n");
        printf ("bBusAddr         = 0x%02x\n", io [0x1ec8]);
        printf ("bMasterForceConf = 0x%02x\n", io [0x1eca]);
        printf ("bDpv1C1BufLen    = 0x%02x\n", io [0x1ecb]);
        printf ("bDpv1C2BufLen    = 0x%02x\n", io [0x1ecc]);
        printf ("bMiscUserFlags   = 0x%02x\n", io [0x1ecd]);
        printf ("bMiscTaskFlags   = 0x%02x\n", io [0x1ece]);
        printf ("\n");
	
        printf ("Module:\n");
        printf ("bBufferLen       = 0x%02x\n", io [0x1ecf]);
        for (i = 0; i < io [0x1ecf]/2; i++)
	{
		printf ("Module [%d]:\n", i);
		printf ("bType = 0x%02x\n", io [0x1ed0 + 2*i]);
		printf ("bLen  = 0x%02x\n", io [0x1ed1 + 2*i]);
		printf ("\n");
	}
	
        printf ("Protocol State:\n");
        printf ("Baud Rate        = %d\n", (io [0x1f41] << 8) + io [0x1f40]);
        printf ("Bus Address      = %d\n", io [0x1f42]);
        printf ("Id Number        = %d.%d\n", io [0x1f43], io [0x1f44]);
        printf ("Task State       = %04x\n", (io [0x1f46] << 8) + io [0x1f45]);
        printf ("Input Len        = %d\n", io [0x1f47]);
        printf ("Output Len       = %d\n", io [0x1f49]);
        printf ("Error Count      = %d\n", (io [0x1f4c] << 8) + io [0x1f4b]);
        printf ("Last Error       = %d\n", io [0x1f4d]);
        printf ("DevTab Entries   = %d\n", io [0x1f5b]);
        printf ("Module Count     = %d\n", io [0x1f5c]);
        printf ("IO Length        = %d\n", io [0x1f5d]);
        printf ("ID Length        = %d\n", io [0x1f5f]);
        printf ("Param Length     = %d\n", io [0x1f60]);
        printf ("Diag Length      = %d\n", io [0x1f61]);
        printf ("\n");
	
	return 0;
}

/**
 * cif_dps_print_image - show DP slave process image 
 */

int cif_dps_print_image (struct cif *cif)
{
	uint8_t *io;
	int i, t;
	
	if (!cif)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	
	printf ("Input:\n");
//	for (i = 0; i < 0x0e00; i += 0x20)
	for (i = 0; i < 0x020; i += 0x20)
	{
		printf ("%04x: ", i);
		for (t = 0; t < 0x20; t++)
			printf ("%02x", io [i + t]);
		printf ("\n");
	}
	
	printf ("Output:\n");
//	for (i = 0x0e00; i < 0x1700; i += 0x20)
	for (i = 0x0e00; i < 0x0e20; i += 0x20)
	{
		printf ("%04x: ", i);
		for (t = 0; t < 0x20; t++)
			printf ("%02x", io [i + t]);
		printf ("\n");
	}
	
	return 0;
}

/**
 * cif_dps_get_plc_mode - get DP slave PLC mode
 */

int cif_dps_get_plc_mode (struct cif *cif, uint8_t *mode)
{
	uint8_t *io;
	
	if (!cif)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	*mode = io [0x1ec0];
	
	return 0;
}

/**
 * cif_dps_set_plc_mode - set DP slave PLC mode
 */

int cif_dps_set_plc_mode (struct cif *cif, uint8_t mode)
{
	uint8_t *io;
	
	if (!cif || mode > 3)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	io [0x1ec0] = mode;
	
	return 0;
}

/**
 * cif_dps_get_wd_time - get DP slave watchdog timeout
 */
int cif_dps_get_wd_time (struct cif *cif, struct timespec *timeout)
{
	uint8_t *io;
	uint16_t l;
	
	if (!cif | !timeout)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	l = *((uint16_t *) (io + 0x1ec1));
	
	timeout->tv_sec = 0;
	timeout->tv_nsec = l*1000;
	
	while (timeout->tv_nsec > 1000000000)
	{
		timeout->tv_nsec -= 1000000000;
		timeout->tv_sec++;
	}
	
	return 0;
}

/**
 * cif_dps_set_plc_mode - set DP slave watchdog timeout
 */

int cif_dps_set_wd_time (struct cif *cif, struct timespec *timeout)
{
	uint8_t *io;
	uint16_t l;
	
	if (!cif | !timeout)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	
	l = timeout->tv_sec*1000000 + timeout->tv_nsec/1000;
	
	*((uint16_t *) (io + 0x1ec1)) = l;
	
	return 0;
}

/**
 * cif_dps_get_bus_addr - get DP slave bus address
 */

uint8_t cif_dps_get_bus_addr (struct cif *cif)
{
	uint8_t *io;
	
	if (!cif)
		return EINVAL;
	io = (uint8_t *)cif_get_io (cif);
	return io [0x1ec8];
}

/**
 * cif_dps_set_bus_addr - set DP slave bus address
 */

int cif_dps_set_bus_addr (struct cif *cif, uint8_t addr)
{
	uint8_t *io;
	
	if (!cif || addr > 125)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	io [0x1ec8] = addr;
	
	return 0;
}

/**
 * cif_dps_get_mod_conf - get DP bus mode
 */
int cif_dps_get_mod_conf (struct cif *cif, unsigned int module, 
			  enum cif_dps_mode *mode, enum cif_dps_elem_len *len)
{
	uint8_t *io;
	
	if (!cif || module > 23 || !mode || !len)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	
	if (module > io [0x1ecf]/2) 
	{
		*mode = CIF_DPS_NOT_VALID;
		*len = CIF_DPS_ELEM_NOT_VALID;
		return -EFAULT;
	}
	
	*mode = (cif_dps_mode)     io [0x1ed0 + 2*module];
	*len =  (cif_dps_elem_len) io [0x1ed1 + 2*module];
	
	return 0;
}

/**
 * cif_dps_set_mod_conf -  set DP bus mode
 */
int cif_dps_set_mod_conf (struct cif *cif, unsigned int module, 
			  enum cif_dps_mode mode, enum cif_dps_elem_len len)
{
	uint8_t *io;
	
	if (!cif || module > 23 || mode == CIF_DPS_NOT_VALID || 
	    len == CIF_DPS_ELEM_NOT_VALID)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	
	if (module > io [0x1ecf]/2) 
		return -EFAULT;
	
	io [0x1ed0 + 2*module] = mode;
	io [0x1ed1 + 2*module] = len;
	
	return 0;
}

/**
 * cif_dps_get_baudrate - get DP slave baudrate
 */
int cif_dps_get_baudrate (struct cif *cif, uint16_t *baudrate)
{
	uint8_t *io;
	
	if (!cif || !baudrate)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	
	*baudrate = (io [0x1f41] << 8) + io [0x1f40];
	
	return 0;
}

/**
 * cif_dps_set_baudrate -  set DP slave baudrate
 */
int cif_dps_set_baudrate (struct cif *cif, uint16_t baudrate)
{
	uint8_t *io;
	
	if (!cif)
		return -EINVAL;
	
	io = (uint8_t *)cif_get_io (cif);
	
	io [0x1f41] = baudrate >> 8;
	io [0x1f40] = baudrate & 0xff;
		     
	return 0;
}


/**
 * cif_dps_activate_config - activate the current configuration
 */

void cif_dps_activate_config (struct cif *cif)
{
	cif_dev_init (cif);
}

/**
 * cif_dps_get_config - get the current configuration
 */

int cif_dps_get_config (struct cif *cif)
{
	uint8_t *io;
	struct cif_msg in, out;
	
	if (!cif)
		return -1;
        
        io = (uint8_t *)cif_get_io (cif);
	
        out.res = 0;
        out.msg.msg_head.rx = 3;
        out.msg.msg_head.tx = 16;
        out.msg.msg_head.ln = 0;
        out.msg.msg_head.nr = 0;
        out.msg.msg_head.a = 0;
        out.msg.msg_head.f = 0;
	out.msg.msg_head.b = 48;
	out.msg.msg_head.e = 0;	
	
	cif_msg_tx (cif, &out, NULL,NULL);
	
	while ((io [0x1fff] & 2) != (io [0x1ffe] & 2))
		usleep (100);
	cif_print_msg (&in);
	
	return 0;
}

/*
 * cif_dps_get_last_error - get last error
 */

int cif_dps_get_last_error (struct cif *cif)
{
	uint8_t *io;
	
	if (!cif)
		return -1;
	
	io = (uint8_t *)cif_get_io (cif);
	return io [0x1f4d];
}
