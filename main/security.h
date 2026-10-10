#ifndef SECURITY_H
#define SECURITY_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void security_init(void);
uint32_t security_get_passkey(void);
void security_set_passkey(uint32_t pk);
void security_get_admin_key(uint8_t *out);
bool security_set_admin_key(const uint8_t *key);
bool security_is_addr_auth(const uint8_t *bd_addr);
void security_set_addr_auth(const uint8_t *bd_addr, bool auth);
void security_remove_addr(const uint8_t *bd_addr);
void security_ota_abrir_ventana(void);
bool security_ota_ventana_abierta(void);
void security_abrir_pairing_ventana(void);
bool security_pairing_ventana_abierta(void);
void security_tick(void);
#endif
