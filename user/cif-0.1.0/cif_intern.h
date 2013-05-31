/* cif_intern.h */
#ifndef CIF_INTERN_H
#define CIF_INTERN_H

struct cif_dev {
	struct cif_dev *next;
	struct cif_dev *prev;
	char *name;
	int io_fd;
	int msg_fd;
	int idx;
};

#endif
