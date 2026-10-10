#include "batch.h"
#include "protocol.h"
#include "ble_manager.h"
#include "crypto.h"
#include "phy_manager.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static struct { uint8_t dest[4]; uint8_t msgs[BATCH_MAX][PAYLOAD_MAX]; uint8_t lens[BATCH_MAX]; uint8_t count; uint32_t ts; bool activo; } s_batch[4];
void batch_init(void) { memset(s_batch, 0, sizeof(s_batch)); }
static int buscar(uint8_t *dest) {
    for (int i = 0; i < 4; i++) if (s_batch[i].activo && memcmp(s_batch[i].dest, dest, 4) == 0) return i;
    return -1;
}
static int crear(uint8_t *dest) {
    for (int i = 0; i < 4; i++)
        if (!s_batch[i].activo) {
            s_batch[i].activo = true;
            memcpy(s_batch[i].dest, dest, 4);
            s_batch[i].ts = xTaskGetTickCount();
            return i;
        }
    return -1;
}
bool batch_add(uint8_t *dest, uint8_t *msg, uint8_t len) {
    if (len > BATCH_LEN) return false;
    int i = buscar(dest); if (i < 0) i = crear(dest); if (i < 0) return false;
    if (s_batch[i].count >= BATCH_MAX) { batch_flush(); i = buscar(dest); if (i < 0) return false; }
    memcpy(s_batch[i].msgs[s_batch[i].count], msg, len);
    s_batch[i].lens[s_batch[i].count] = len;
    s_batch[i].count++;
    s_batch[i].ts = xTaskGetTickCount();
    if (s_batch[i].count >= BATCH_MAX) batch_flush();
    return true;
}
void batch_flush(void) {
    for (int i = 0; i < 4; i++) {
        if (!s_batch[i].activo || s_batch[i].count == 0) continue;
        uint8_t buf[PAYLOAD_MAX]; uint8_t pos = 0;
        buf[pos++] = s_batch[i].count;
        for (int j = 0; j < s_batch[i].count && pos < PAYLOAD_MAX - 1; j++) {
            buf[pos++] = s_batch[i].lens[j];
            if (pos + s_batch[i].lens[j] > PAYLOAD_MAX) break;
            memcpy(&buf[pos], s_batch[i].msgs[j], s_batch[i].lens[j]);
            pos += s_batch[i].lens[j];
        }
        packet_t p = {0};
        p.header = (ROUTE_FLOOD << 6) | (PAYLOAD_BATCH << 2) | ((TTL_MAX-1) << 4) | 0x01;
        p.msg_id = (uint8_t)esp_random();
        p.path_len = 4;
        memcpy(p.path, proto_get_id(), 4);
        int enc = crypto_encrypt(buf, pos, p.nonce);
        p.payload_len = enc + 4;
        memcpy(p.payload, buf, enc);
        memcpy(&p.payload[enc], s_batch[i].dest, 4);
        ble_emitir((uint8_t*)&p, sizeof(p), phy_actual());
        s_batch[i].count = 0; s_batch[i].activo = false;
    }
}
void batch_expirar(void) {
    uint32_t ahora = xTaskGetTickCount();
    for (int i = 0; i < 4; i++)
        if (s_batch[i].activo && (ahora - s_batch[i].ts) > pdMS_TO_TICKS(2000))
            batch_flush();
}
