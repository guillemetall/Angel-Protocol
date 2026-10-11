#ifndef BRIDGE_H
#define BRIDGE_H
#include <stdint.h>
#include <stdbool.h>
void bridge_init(void);
void bridge_set_activo(bool on);
bool bridge_activo(void);
void bridge_iphone_rx(uint8_t *data, uint16_t len);
void bridge_mesh_rx(uint8_t *data, uint16_t len);
void bridge_notify_ack(void);
#endif
