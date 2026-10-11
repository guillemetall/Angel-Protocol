/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "congestion.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TOKEN_RATE  10
#define TOKEN_MAX   20

static uint32_t s_airtime_us = 0;
static uint32_t s_ventana_inicio = 0;
static bool     s_urgente = false;
static uint8_t  s_nivel = 0;
static bool     s_transfiriendo = false;
static uint32_t s_tokens = TOKEN_MAX;
static uint32_t s_ultimo_relleno = 0;

void congestion_init(void) {
    s_airtime_us = 0;
    s_ventana_inicio = xTaskGetTickCount();
    s_urgente = false;
    s_nivel = 0;
    s_transfiriendo = false;
    s_tokens = TOKEN_MAX;
    s_ultimo_relleno = xTaskGetTickCount();
}

void congestion_marcar_tx(uint32_t airtime_us) {
    s_airtime_us += airtime_us;
}

static void actualizar(void) {
    uint32_t ahora = xTaskGetTickCount();
    uint32_t elapsed = (ahora - s_ventana_inicio) * 1000;
    if (elapsed >= CONGESTION_VENTANA * 1000) {
        s_nivel = (uint8_t)((s_airtime_us * 100) / (CONGESTION_VENTANA * 1000));
        if (s_nivel > 100) s_nivel = 100;
        s_airtime_us = 0;
        s_ventana_inicio = ahora;
    }
}

static void rellenar_tokens(void) {
    uint32_t ahora = xTaskGetTickCount();
    uint32_t delta_ms = (ahora - s_ultimo_relleno) * portTICK_PERIOD_MS;
    uint32_t nuevos = (delta_ms * TOKEN_RATE) / 1000;
    if (nuevos > 0) {
        s_tokens += nuevos;
        if (s_tokens > TOKEN_MAX) s_tokens = TOKEN_MAX;
        s_ultimo_relleno = ahora;
    }
}

uint8_t congestion_nivel(void) { actualizar(); return s_nivel; }

bool congestion_puede_enviar(void) {
    actualizar();
    return s_urgente || s_transfiriendo || s_nivel < CONGESTION_UMBRAL;
}

bool congestion_es_urgente(void) { return s_urgente; }
void congestion_set_urgente(bool u) { s_urgente = u; }
void congestion_set_transfiriendo(bool on) { s_transfiriendo = on; }
bool congestion_esta_transfiriendo(void) { return s_transfiriendo; }

bool congestion_tiene_token(void) {
    rellenar_tokens();
    return s_tokens > 0;
}

void congestion_consumir_token(void) {
    if (s_tokens > 0) s_tokens--;
}

uint8_t congestion_tokens_actuales(void) {
    rellenar_tokens();
    return (uint8_t)s_tokens;
}
