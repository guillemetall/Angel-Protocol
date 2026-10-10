#include "dedup.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static struct { uint32_t hash; uint32_t ts; bool usado; } s_cache[DEDUP_CACHE];
void dedup_init(void) { memset(s_cache, 0, sizeof(s_cache)); }
uint32_t dedup_hash_pkt(uint8_t *data, uint16_t len) {
    uint32_t h = 2166136261u;
    for (uint16_t i = 0; i < len; i++) { h ^= data[i]; h *= 16777619u; }
    return h;
}
bool dedup_visto(uint32_t hash) {
    uint32_t ahora = xTaskGetTickCount();
    for (int i = 0; i < DEDUP_CACHE; i++)
        if (s_cache[i].usado && s_cache[i].hash == hash) {
            if ((ahora - s_cache[i].ts) < pdMS_TO_TICKS(60000)) return true;
            s_cache[i].usado = false;
        }
    return false;
}
void dedup_marcar(uint32_t hash) {
    uint32_t ahora = xTaskGetTickCount();
    int idx = 0; uint32_t mv = 0xFFFFFFFF;
    for (int i = 0; i < DEDUP_CACHE; i++) {
        if (!s_cache[i].usado) { idx = i; goto llenar; }
        if (s_cache[i].ts < mv) { mv = s_cache[i].ts; idx = i; }
    }
llenar:
    s_cache[idx].hash = hash; s_cache[idx].ts = ahora; s_cache[idx].usado = true;
}
