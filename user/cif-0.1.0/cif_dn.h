/* DeviceNet slave */
#ifndef CIF_DN_H
#define CIF_DN_H

int cif_dn_info (struct cif *cif);
int cif_dn_print_image (struct cif *cif);
int cif_dn_get_plc_mode (struct cif *cif, uint8_t *mode);
int cif_dn_set_plc_mode (struct cif *cif, uint8_t mode);
int cif_dn_get_wd_time (struct cif *cif, struct timespec *timeout);
int cif_dn_set_wd_time (struct cif *cif, struct timespec *timeout);
int cif_dn_get_prod_size (struct cif *cif, uint16_t *psize);
int cif_dn_set_prod_size (struct cif *cif, uint16_t psize);
int cif_dn_get_cons_size (struct cif *cif, uint16_t *csize);
int cif_dn_set_cons_size (struct cif *cif, uint16_t csize);
int cif_dn_set_mac_id (struct cif *cif, uint8_t addr);
int cif_dn_get_mac_id (struct cif *cif, uint8_t *mac);
int cif_dn_get_baudrate (struct cif *cif, uint16_t *baudrate);
int cif_dn_set_baudrate (struct cif *cif, uint16_t baudrate);
void cif_dn_activate_config (struct cif *cif);
int cif_dn_get_last_error (struct cif *cif);

#endif
