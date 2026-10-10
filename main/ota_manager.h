#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H
#include <stdint.h>
#include <stdbool.h>
void ota_handle_write(uint8_t *data, uint16_t len);
void ota_wifi_start(void);
void ota_set_sha256(const uint8_t *sha);
bool ota_verificar_sha256(void);
void ota_modo_recuperacion(void);
#endif
