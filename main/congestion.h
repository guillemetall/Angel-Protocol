#ifndef CONGESTION_H
#define CONGESTION_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void congestion_init(void);
void congestion_marcar_tx(uint32_t airtime_us);
uint8_t congestion_nivel(void);
bool congestion_puede_enviar(void);
bool congestion_es_urgente(void);
void congestion_set_urgente(bool u);
#endif
