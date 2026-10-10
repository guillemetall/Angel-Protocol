#include "congestion.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static uint32_t s_airtime_us = 0;
static uint32_t s_ventana_inicio = 0;
static bool s_urgente = false;
static uint8_t s_nivel = 0;
void congestion_init(void) { s_airtime_us = 0; s_ventana_inicio = xTaskGetTickCount(); }
void congestion_marcar_tx(uint32_t airtime_us) { s_airtime_us += airtime_us; }
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
uint8_t congestion_nivel(void) { actualizar(); return s_nivel; }
bool congestion_puede_enviar(void) { actualizar(); return s_urgente || s_nivel < CONGESTION_UMBRAL; }
bool congestion_es_urgente(void) { return s_urgente; }
void congestion_set_urgente(bool u) { s_urgente = u; }
