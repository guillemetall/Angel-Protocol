#include "fragment.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static struct { uint8_t src[4]; uint8_t msg_id; uint8_t total; uint8_t recibidos; uint32_t bitmap; uint8_t buffer[FRAG_MAX * FRAG_CHUNK]; uint32_t ts; bool activo; } s_ctx[4];
static uint8_t s_next_id = 0;
void frag_init(void) { memset(s_ctx, 0, sizeof(s_ctx)); }
static int buscar_ctx(uint8_t *src, uint8_t msg_id) {
    for (int i = 0; i < 4; i++)
        if (s_ctx[i].activo && s_ctx[i].msg_id == msg_id && memcmp(s_ctx[i].src, src, 4) == 0) return i;
    return -1;
}
static int nuevo_ctx(uint8_t *src, uint8_t msg_id, uint8_t total) {
    for (int i = 0; i < 4; i++)
        if (!s_ctx[i].activo) {
            s_ctx[i].activo = true;
            memcpy(s_ctx[i].src, src, 4);
            s_ctx[i].msg_id = msg_id; s_ctx[i].total = total;
            s_ctx[i].recibidos = 0; s_ctx[i].bitmap = 0;
            s_ctx[i].ts = xTaskGetTickCount();
            return i;
        }
    return -1;
}
bool frag_enviar(uint8_t *dest, uint8_t *data, uint16_t len, frag_send_cb_t cb) {
    if (len <= FRAG_CHUNK) return cb(data, len, 0);
    uint8_t total = (len + FRAG_CHUNK - 1) / FRAG_CHUNK;
    if (total > FRAG_MAX) return false;
    uint8_t msg_id = s_next_id++;
    bool ok = true;
    for (uint8_t i = 0; i < total; i++) {
        uint8_t buf[FRAG_CHUNK + 4];
        buf[0] = msg_id; buf[1] = total; buf[2] = i;
        uint16_t chunk = (i == total - 1) ? (len - i * FRAG_CHUNK) : FRAG_CHUNK;
        memcpy(&buf[3], &data[i * FRAG_CHUNK], chunk);
        if (!cb(buf, chunk + 3, 1)) ok = false;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    return ok;
}
bool frag_recibir(uint8_t *src, uint8_t *data, uint16_t len, uint8_t *out, uint16_t *out_len) {
    if (len < 3) return false;
    uint8_t msg_id = data[0]; uint8_t total = data[1]; uint8_t idx = data[2];
    uint16_t chunk = len - 3;
    if (chunk > FRAG_CHUNK) return false;   /* fix Bug 6: chunk oversized */
    if (total > FRAG_MAX || idx >= total) return false;
    int i = buscar_ctx(src, msg_id);
    if (i < 0) i = nuevo_ctx(src, msg_id, total);
    if (i < 0) return false;
    if (s_ctx[i].bitmap & (1u << idx)) return false;
    memcpy(&s_ctx[i].buffer[idx * FRAG_CHUNK], &data[3], chunk);
    s_ctx[i].bitmap |= (1u << idx); s_ctx[i].recibidos++;
    s_ctx[i].ts = xTaskGetTickCount();
    if (s_ctx[i].recibidos == s_ctx[i].total) {
        uint16_t total_len = (s_ctx[i].total - 1) * FRAG_CHUNK + chunk;
        if (total_len > FRAG_MAX * FRAG_CHUNK) return false;   /* fix Bug 6b: cap salida */
        memcpy(out, s_ctx[i].buffer, total_len);
        *out_len = total_len;
        s_ctx[i].activo = false;
        return true;
    }
    return false;
}
void frag_expirar(void) {
    uint32_t ahora = xTaskGetTickCount();
    for (int i = 0; i < 4; i++)
        if (s_ctx[i].activo && (ahora - s_ctx[i].ts) > pdMS_TO_TICKS(30000))
            s_ctx[i].activo = false;
}
