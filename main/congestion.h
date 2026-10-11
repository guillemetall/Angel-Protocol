/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CONGESTION_H
#define CONGESTION_H
#include <stdint.h>
#include <stdbool.h>
void congestion_init(void);
void congestion_marcar_tx(uint32_t airtime_us);
uint8_t congestion_nivel(void);
bool congestion_puede_enviar(void);
bool congestion_es_urgente(void);
void congestion_set_urgente(bool u);
void congestion_set_transfiriendo(bool on);
bool congestion_esta_transfiriendo(void);
bool congestion_tiene_token(void);
void congestion_consumir_token(void);
uint8_t congestion_tokens_actuales(void);
