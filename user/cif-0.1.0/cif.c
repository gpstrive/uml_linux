/*
 * cif - new API for Hilscher CIF cards
 *
 * Copyright (C) 2006 Benedikt Spranger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <argp.h>
#include <errno.h>
#include <error.h>
#include <stdint.h>
#include "system.h"
//#include "cif-opt.h"
#include "cif.h"
// #include "cif_dps.h"

struct cif *cif;

static void show_mem_16(FILE *stream, uint8_t *mem, int start_addr)
{
	int i;
	unsigned char ch;
	fprintf(stream,"%04X",start_addr);
	for (i=0; i<16; i++)
		fprintf(stream," %02X",(int)mem[start_addr+i]);
	fprintf(stream," ");
	for (i=0; i<16; i++) {
		ch = mem[start_addr+i];
		fprintf(stream,"%c",(isprint(ch)) ? ch : '.');
	}
	fprintf(stream,"\n");
}

int main (int argc, char **argv)
{
	int i;
	uint8_t *io, *io_plx;
	struct cif_msg in, out;

	cif = cif_open (argv[1], 0);

	if (!cif)
	{
		perror ("cif_open");
		exit(1);
	}

	printf("cif_open(%s) succeeded.\n", argv[1]);
	fflush(stdout);

	io = cif_get_io (cif);
	printf("cif_get_io() returns 0x%08X\n",(int)io);
	io_plx = cif_get_io_plx (cif);
	//printf("host: 0x%02X dev: 0x%02X\n",(int)io[0x1ffe],(int)io[0x1fff]);
	/*
	for (i=8192-1024; i<8192; i += 16) {
		show_mem_16(stdout,io,i);
	}
	fflush(stdout);
	*/
	printf ("***\n");

	if (cif_dev_init (cif)) {
		printf("cif_dev_init() failed.\n");
		cif_close (cif);
		exit(1);
	}

	printf("CIF UIO device opened and initialized.\n");
	fflush(stdout);

	cif_print_info (cif);

	printf ("***\n");

	//printf (">%d\n", cif_conf_upload (cif, "/root/cpx.dbm"));

	io_plx[0x4c] |= 0x01; /* Enable Interrupts */

	printf("Will close soon... ");
	fflush(stdout);
	sleep(3);

	cif_close (cif);
	printf("Ready.\n");
	exit(0);
}
