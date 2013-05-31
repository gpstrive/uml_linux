/* Profibus DP slave */
#ifndef CIF_DPS_H
#define CIF_DPS_H
typedef enum cif_dps_mode
{
	CIF_DPS_NOT_VALID		= -1,
	CIF_DPS_IN_BYTE_NO_CHECK	= 0,
	CIF_DPS_IN_WORD_NO_CHECK	= 1,
	CIF_DPS_OUT_BYTE_NO_CHECK	= 2,
	CIF_DPS_OUT_WORD_NO_CHECK	= 3,
	CIF_DPS_IN_BYTE_CHECK		= 4,
	CIF_DPS_IN_WORD_CHECK		= 5,
	CIF_DPS_OUT_BYTE_CHECK		= 6,
	CIF_DPS_OUT_WORD_CHECK		= 7,
	CIF_DPS_DUMMY			= 8,
} cif_dps_mode ;

typedef enum cif_dps_elem_len
{
	CIF_DPS_ELEM_NOT_VALID		= -1,
	CIF_DPS_ELEM_LEN_1		= 0,
	CIF_DPS_ELEM_LEN_2		= 1,
	CIF_DPS_ELEM_LEN_3		= 2,
	CIF_DPS_ELEM_LEN_4		= 3,
	CIF_DPS_ELEM_LEN_8		= 4,
	CIF_DPS_ELEM_LEN_12		= 5,
	CIF_DPS_ELEM_LEN_16		= 6,
	CIF_DPS_ELEM_LEN_20		= 7,
	CIF_DPS_ELEM_LEN_32		= 8,
	CIF_DPS_ELEM_LEN_64		= 9,
} cif_dps_elem_len ;

int cif_dps_info (struct cif *cif);
int cif_dps_print_image (struct cif *cif);
int cif_dps_get_plc_mode (struct cif *cif, uint8_t *mode);
int cif_dps_set_plc_mode (struct cif *cif, uint8_t mode);
int cif_dps_get_wd_time (struct cif *cif, struct timespec *timeout);
int cif_dps_set_wd_time (struct cif *cif, struct timespec *timeout);
uint8_t cif_dps_get_bus_addr (struct cif *cif);
int cif_dps_set_bus_addr (struct cif *cif, uint8_t addr);
int cif_dps_get_mod_conf (struct cif *cif, unsigned int module,
			  enum cif_dps_mode *mode, enum cif_dps_elem_len *len);
int cif_dps_set_mod_conf (struct cif *cif, unsigned int module,
			  enum cif_dps_mode mode, enum cif_dps_elem_len len);
int cif_dps_get_baudrate (struct cif *cif, uint16_t *baudrate);
int cif_dps_set_baudrate (struct cif *cif, uint16_t baudrate);
void cif_dps_activate_config (struct cif *cif);
int cif_dps_get_last_error (struct cif *cif);

#endif
