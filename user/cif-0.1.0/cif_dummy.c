#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cif.h"
#include "cif_intern.h"

int cif_test_hardware (struct cif *cif);
int cif_test_dpm_access (struct cif *cif);
int cif_test_rdy_flag (struct cif *cif);

int cif_test_hardware (struct cif *cif)
{
	int ret = -1;
	
	if (!cif)
		goto out;
	
	ret = cif_test_dpm_access (cif);
	if (ret)
		goto out;
	
	ret = cif_test_rdy_flag (cif);
		
out:
	return ret;
}

const uint8_t *cif_dev_identifier[] = 
{
	"CIF",
	"COM",
	"COP",
	"DEV",
	NULL
};

const uint8_t cif_test_identifier [4] = {0x55, 0xAA, 0x00, 0x00};

int cif_test_dpm_access (struct cif *cif)
{
	int ret = -1, id_ok = 0;
	uint8_t id [4];
	uint8_t *io, **tmp;
	
	if (!cif)
		goto out;
	
	io = cif_get_io (cif);
	memcpy (id, io + 0x1ffb, 3);
	
	for (tmp = cif_dev_identifier; *tmp; tmp++)
		if (!memcmp (id, *tmp, 3))
			id_ok = 1;
	if (!id_ok)
		goto out;
	
	memcpy (io + 0x1ffb, cif_test_identifier, 3);
	if (memcmp (io + 0x1ffb, cif_test_identifier, 3))
		goto out;
	
	memcpy (io + 0x1ffb, id, 3);
	if (!memcmp (io + 0x1ffb, id, 3))
		ret = 0;
out:
	return ret;
}

int cif_test_rdy_flag (struct cif *cif)
{
	int ret = -1;
	if (!cif)
		goto out;
	
	if (cif->hostflags & 0x80)
		ret = 0;
out:
	return ret;
}

