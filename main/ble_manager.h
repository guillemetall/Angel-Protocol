#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H
#include <stdint.h>
#include <stdbool.h>
void ble_init(void);
bool ble_emitir(uint8_t *data, uint16_t len, uint8_t phy);
void ble_scan_start(void);
void ble_send(uint8_t *data, uint16_t len);
typedef void (*ble_msg_cb_t)(uint8_t *data, uint16_t len);
void ble_set_msg_cb(ble_msg_cb_t cb);
#endif
