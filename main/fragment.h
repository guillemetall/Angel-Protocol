#ifndef FRAGMENT_H
#define FRAGMENT_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void frag_init(void);
typedef bool (*frag_send_cb_t)(uint8_t *data, uint16_t len, uint8_t flags);
bool frag_enviar(uint8_t *dest, uint8_t *data, uint16_t len, frag_send_cb_t cb);
bool frag_recibir(uint8_t *src, uint8_t *data, uint16_t len, uint8_t *out, uint16_t *out_len);
void frag_expirar(void);
#endif
