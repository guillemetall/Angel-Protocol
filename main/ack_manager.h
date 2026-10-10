#ifndef ACK_MANAGER_H
#define ACK_MANAGER_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void ack_init(void);
void ack_registrar(uint8_t *src, uint8_t seq);
void ack_enviar_cum(uint8_t *dest);
void ack_recibir_cum(uint8_t *src, uint8_t base_seq, uint32_t bitmap);
void ack_expirar(void);
uint8_t ack_proxima_seq(void);
#endif
