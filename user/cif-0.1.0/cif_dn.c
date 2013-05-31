#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "cif.h"
#include "cif_dn.h"

/**
 * cif_dn_info - show DeviceNet slave parameter and status
 */

int cif_dn_info (struct cif *cif)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);

	printf ("DeviceNet slave:\n");

	printf ("WatchDog  = %d ms\n", io [0x1ec3] + (io [0x1ec4] << 8));
	printf ("MAC ID    = %d\n", io [0x1ec5]);
	printf ("Vendor ID = %d\n", io [0x1ec6] + (io [0x1ec7] << 8));
	printf ("Baudrate  = ");
	switch (io [0x1ec8])
	{
	case 0:
		printf ("500k\n");
		break;
	case 1:
		printf ("250k\n");
		break;
	case 2:
		printf ("125k\n");
		break;
	}

	return 0;
}

/**
 * cif_dn_print_image - show DeviceNet slave process image
 */

int cif_dn_print_image (struct cif *cif)
{
	uint8_t *io;
	int i, t;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);

	printf ("Input:\n");
	//      for (i = 0; i < 0x0e00; i += 0x20)
	for (i = 0; i < 0x100; i += 0x20)
	{
		printf ("%04x: ", i);
		for (t = 0; t < 0x20; t++)
			printf ("%02x", io [i + t]);
		printf ("\n");
	}

	printf ("Output:\n");
	//      for (i = 0x0e00; i < 0x1700; i += 0x20)
	for (i = 0x0e00; i < 0x0f00; i += 0x20)
	{
		printf ("%04x: ", i);
		for (t = 0; t < 0x20; t++)
			printf ("%02x", io [i + t]);
		printf ("\n");
	}

	return 0;
}

/**
 * cif_dn_get_plc_mode - get DP slave PLC mode
 */

int cif_dn_get_plc_mode (struct cif *cif, uint8_t *mode)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	*mode = io [0x1ec0];

	return 0;
}

/**
 * cif_dn_set_plc_mode - set DP slave PLC mode
 */

int cif_dn_set_plc_mode (struct cif *cif, uint8_t mode)
{
	uint8_t *io;

	if (!cif || mode > 3)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	io [0x1ec0] = mode;

	return 0;
}

/**
 * cif_dn_get_prod_size - get DP produced size
 */

int cif_dn_get_prod_size (struct cif *cif, uint16_t *psize)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *) cif_get_io (cif);
	*psize = io [0x1ec9] + (io [0x1eca] << 8);

	return 0;
}

/**
 * cif_dn_set_prod_size - set DP produced size
 */

int cif_dn_set_prod_size (struct cif *cif, uint16_t psize)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	io [0x1ec9] = psize & 0xff;
	io [0x1eca] = psize >> 8;

	return 0;
}

/**
 * cif_dn_get_cons_size - get DP consumed size
 */

int cif_dn_get_cons_size (struct cif *cif, uint16_t *csize)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *) cif_get_io (cif);
	*csize = io [0x1ecb] + (io [0x1ecc] << 8);

	return 0;
}

/**
 * cif_dn_set_cons_size - set DP consumed size
 */

int cif_dn_set_cons_size (struct cif *cif, uint16_t csize)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	io [0x1ecb] = csize & 0xff;
	io [0x1ecc] = csize >> 8;

	return 0;
}

/**
 * cif_dn_get_mac_id - get DeviceNet slave MAC ID
 */

int cif_dn_get_mac_id (struct cif *cif, uint8_t *mac)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	*mac = io [0x1ec5];

	return 0;
}

/**
 * cif_dn_set_mac_id - get DeviceNet slave MAC ID
 */

int cif_dn_set_mac_id (struct cif *cif, uint8_t mac)
{
	uint8_t *io;

	if (!cif || mac > 63)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	io [0x1ec5] = mac;

	return 0;
}

/**
 * cif_dn_get_vendor_id - get DeviceNet slave Vendor ID
 */

int cif_dn_get_vendor_id (struct cif *cif, uint16_t *vid)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	*vid = io [0x1ec6] + (io [0x1ec7] << 8);

	return 0;
}

/**
 * cif_dn_set_vendor_id - get DeviceNet slave Vendor ID
 */

int cif_dn_set_vendor_id (struct cif *cif, uint16_t vid)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);
	io [0x1ec6] = vid & 0xff;
	io [0x1ec7] = vid >> 8;

	return 0;
}

/**
 * cif_dn_get_baudrate - get DeviceNet slave Baudrate
 */

int cif_dn_get_baudrate (struct cif *cif, uint16_t *baud)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);

	switch (io [0x1ec8])
	{
	case 0:
		*baud = 500;
		break;
	case 1:
		*baud = 250;
		break;
	case 2:
		*baud = 125;
		break;
	default:
		*baud = 0;
	}

	return 0;
}

/**
 * cif_dn_set_baudrate - set DeviceNet slave Baudrate
 */

int cif_dn_set_baudrate (struct cif *cif, uint16_t baud)
{
	uint8_t *io;

	if (!cif)
		return -EINVAL;

	io = (uint8_t *)cif_get_io (cif);

	switch (baud)
	{
	case 500:
		io [0x1ec8] = 0;
		break;
	case 250:
		io [0x1ec8] = 1;
		break;
	case 125:
		io [0x1ec8] = 2;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

void cif_dn_activate_config (struct cif *cif)
{
  cif_dev_init (cif);
}
