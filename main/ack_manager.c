#include "ack_manager.h"
#include "protocol.h"
#include "ble_manager.h"
#include "crypto.h"
#include "phy_manager.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static struct { uint8_t dest[4]; uint32_t bitmap; uint8_t base_seq; uint32_t ts; bool activo; } s_ack[8];
static uint8_t s_seq = 0;
void ack_init(void) { memset(s_ack, 0, sizeof(s_ack)); s_seq = 0; }
uint8_t ack_proxima_seq(void) { return s_seq++; }
static int buscar(uint8_t *dest) {
    for (int i = 0; i < 8; i++) if (s_ack[i].activo && memcmp(s_ack[i].dest, dest, 4) == 0) return i;
    return -1;
}
static int crear(uint8_t *dest) {
    for (int i = 0; i < 8; i++)
        if (!s_ack[i].activo) {
            s_ack[i].activo = true;
            memcpy(s_ack[i].dest, dest, 4);
            s_ack[i].ts = xTaskGetTickCount();
            return i;
        }
    return -1;
}
void ack_registrar(uint8_t *src, uint8_t seq) {
    int i = buscar(src); if (i < 0) i = crear(src); if (i < 0) return;
    uint8_t off = seq - s_ack[i].base_seq;
    if (off < 32) s_ack[i].bitmap |= (1u << off);
    s_ack[i].ts = xTaskGetTickCount();
}
void ack_enviar_cum(uint8_t *dest) {
    int i = buscar(dest); if (i < 0) return;
    packet_t p = {0};
    p.header = (ROUTE_DIRECT << 6) | (PAYLOAD_ACK_CUM << 2) | ((TTL_MAX-1) << 4) | 0x01;
    p.msg_id = (uint8_t)esp_random();
    p.path_len = 4;
    memcpy(p.path, dest, 4);
    uint8_t buf[5] = { s_ack[i].base_seq, (uint8_t)(s_ack[i].bitmap & 0xFF),
        (uint8_t)((s_ack[i].bitmap >> 8) & 0xFF), (uint8_t)((s_ack[i].bitmap >> 16) & 0xFF),
        (uint8_t)((s_ack[i].bitmap >> 24) & 0xFF) };
    int enc = crypto_encrypt(buf, 5, p.nonce);
    p.payload_len = enc + 4;
    memcpy(p.payload, buf, enc);
    memcpy(&p.payload[enc], dest, 4);
    ble_emitir((uint8_t*)&p, sizeof(p), phy_actual());
    s_ack[i].bitmap = 0;
    s_ack[i].base_seq = s_seq;
}
void ack_recibir_cum(uint8_t *src, uint8_t base_seq, uint32_t bitmap) {
    int i = buscar(src); if (i < 0) i = crear(src); if (i < 0) return;
    s_ack[i].base_seq = base_seq;
    s_ack[i].bitmap = bitmap;
    s_ack[i].ts = xTaskGetTickCount();
}
void ack_expirar(void) {
    uint32_t ahora = xTaskGetTickCount();
    for (int i = 0; i < 8; i++)
        if (s_ack[i].activo && (ahora - s_ack[i].ts) > pdMS_TO_TICKS(60000))
            s_ack[i].activo = false;
}
