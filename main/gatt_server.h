#ifndef GATT_SERVER_H
#define GATT_SERVER_H
#include <stdint.h>
void gatt_server_init(void);
void gatt_notify_status(uint8_t *data, uint16_t len);
void gatt_notify_msg(uint8_t *data, uint16_t len);
void gatt_cargar_cfg(void);
#endif
