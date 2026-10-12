#warning HARDENING_TEST_FLAG
#include "protocol.h"
#include "phy_manager.h"
#include "ble_manager.h"
#include "gatt_server.h"
#include "power_manager.h"
#include "crypto.h"
#include "dedup.h"
#include "fragment.h"
#include "room.h"
#include "ack_manager.h"
#include "batch.h"
#include "congestion.h"
#include "bridge.h"
#include "low_priority.h"
#include "fragment.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static vecino_t s_vecinos[MAX_VECINOS];
static uint8_t s_frag_dest[4] = {0};
static uint8_t s_id[4];
static node_mode_t s_modo = MODE_BRIDGE;
static uint32_t s_tx = 0, s_rx = 0;
#define P_RSSI 0.40f
#define P_PDR 0.30f
#define P_BAT 0.10f
#define P_HOP 0.20f
void proto_init(void) {
    esp_fill_random(s_id, 4);
    memset(s_vecinos, 0, sizeof(s_vecinos));
    dedup_init(); frag_init(); room_init(); ack_init(); batch_init(); congestion_init();
}
void proto_set_modo(node_mode_t m) { s_modo = m; }
void proto_set_modo_phy(phy_mode_t m) { phy_set_modo(m); }
void proto_set_modo_nodo(node_mode_t m) { s_modo = m; }
uint8_t* proto_get_id(void) { return s_id; }
int proto_get_vecinos(vecino_t **out) { *out = s_vecinos; return MAX_VECINOS; }
static float norm_rssi(int8_t r) { float n = (r + 100) / 50.0f; return (n < 0) ? 0 : (n > 1) ? 1 : n; }
static float calc_score(vecino_t *v) {
    return P_RSSI*norm_rssi(v->rssi) + P_PDR*v->pdr + P_BAT*(v->bateria/100.0f) + P_HOP*(1.0f/(1+v->saltos_estimados));
}
static void recalcular(void) {
    for (int i = 0; i < MAX_VECINOS; i++) if (s_vecinos[i].activo) s_vecinos[i].score = calc_score(&s_vecinos[i]);
}
static vecino_t* elegir_salto(uint8_t *destino) {
    (void)destino;
    vecino_t *mejor = NULL; float ms = 0;
    for (int i = 0; i < MAX_VECINOS; i++) {
        if (!s_vecinos[i].activo) continue;
        if (s_vecinos[i].rssi < -95) continue;
        if (s_vecinos[i].pdr < 0.3f && s_vecinos[i].envios > 5) continue;
        if (s_vecinos[i].score > ms) { ms = s_vecinos[i].score; mejor = &s_vecinos[i]; }
    }
    return mejor;
}
static void add_vecino(uint8_t *id, int8_t rssi) {
    for (int i = 0; i < MAX_VECINOS; i++)
        if (s_vecinos[i].activo && memcmp(s_vecinos[i].id, id, 4) == 0) {
            s_vecinos[i].rssi = (int8_t)(0.3f * rssi + 0.7f * s_vecinos[i].rssi);
            s_vecinos[i].visto = xTaskGetTickCount();
            s_vecinos[i].score = calc_score(&s_vecinos[i]);
            return;
        }
    for (int i = 0; i < MAX_VECINOS; i++)
        if (!s_vecinos[i].activo) {
            memcpy(s_vecinos[i].id, id, 4);
            s_vecinos[i].rssi = rssi;
            s_vecinos[i].pdr = 1.0f;
            s_vecinos[i].bateria = 100;
            s_vecinos[i].visto = xTaskGetTickCount();
            s_vecinos[i].activo = true;
            s_vecinos[i].score = calc_score(&s_vecinos[i]);
            return;
        }
}
static void reg_ack(uint8_t *id) {
    for (int i = 0; i < MAX_VECINOS; i++)
        if (s_vecinos[i].activo && memcmp(s_vecinos[i].id, id, 4) == 0) {
            s_vecinos[i].acks++;
            if (s_vecinos[i].envios > 0) s_vecinos[i].pdr = (float)s_vecinos[i].acks / s_vecinos[i].envios;
            s_vecinos[i].score = calc_score(&s_vecinos[i]);
            return;
        }
}
static void reg_envio(uint8_t *id) {
    for (int i = 0; i < MAX_VECINOS; i++)
        if (s_vecinos[i].activo && memcmp(s_vecinos[i].id, id, 4) == 0) {
            s_vecinos[i].envios++;
            if (s_vecinos[i].envios > 0) s_vecinos[i].pdr = (float)s_vecinos[i].acks / s_vecinos[i].envios;
            s_vecinos[i].score = calc_score(&s_vecinos[i]);
            return;
        }
}
static void upd_bat(uint8_t *id, uint8_t bat) {
    for (int i = 0; i < MAX_VECINOS; i++)
        if (s_vecinos[i].activo && memcmp(s_vecinos[i].id, id, 4) == 0) {
            s_vecinos[i].bateria = bat;
            s_vecinos[i].score = calc_score(&s_vecinos[i]);
            return;
        }
}
static bool frag_cb(uint8_t *data, uint16_t len, uint8_t flags) {
    (void)flags;
    packet_t p = {0};
    p.header = (ROUTE_FLOOD << 6) | (PAYLOAD_FRAG << 2) | ((TTL_MAX-1) << 4) | 0x01;
    p.msg_id = (uint8_t)esp_random();
    p.path_len = 4;
    memcpy(p.path, s_id, 4);

    uint8_t buf[PAYLOAD_MAX];
    memcpy(buf, data, len);
    int enc = crypto_encrypt(buf, len, p.nonce);
    if (enc < 0) return false;
    p.payload_len = enc;
    memcpy(p.payload, buf, enc);
    memcpy(&p.payload[enc], s_frag_dest, 4);
    p.payload_len = enc + 4;

    if (!congestion_tiene_token()) return false;
    congestion_consumir_token();
    congestion_marcar_tx(3088);
    return low_priority_enqueue((uint8_t*)&p, sizeof(p));
}
static uint8_t s_proto_out[FRAG_MAX * PAYLOAD_MAX];  /* fix Bug 5: movido de pila (.stack) a .bss */
void proto_recibir(uint8_t *data, uint16_t len, int8_t rssi) {
    if (len < 8) return;
    packet_t *p = (packet_t *)data;
    s_rx++;
    phy_actualizar(rssi, true);
    uint32_t h = dedup_hash_pkt(data, len);
    if (dedup_visto(h)) return;
    dedup_marcar(h);
    uint8_t *emisor = NULL;
    if (p->path_len >= 4) {
        emisor = &p->path[p->path_len - 4];
        add_vecino(emisor, rssi);
    }
    uint8_t ptype = (p->header >> 2) & 0x0F;
    if (p->payload_len < 4 + AES_TAG_LEN) return;
    uint8_t enc_len = p->payload_len - 4;
    int dec_len = crypto_decrypt(p->payload, enc_len, p->nonce);
    if (dec_len < 0) return;
    if (ptype == PAYLOAD_ADVERT) { if (dec_len >= 1 && emisor) upd_bat(emisor, p->payload[0]); return; }
    if (ptype == PAYLOAD_ACK) { if (emisor) reg_ack(emisor); bridge_notify_ack(); return; }
    if (ptype == PAYLOAD_ACK_CUM && dec_len >= 5) {
        ack_recibir_cum(emisor, p->payload[0], p->payload[1] | (p->payload[2]<<8) | (p->payload[3]<<16) | ((uint32_t)p->payload[4]<<24));
        return;
    }
    if (ptype == PAYLOAD_BATCH && dec_len >= 1) {
        uint8_t n = p->payload[0]; uint8_t pos = 1;
        for (int i = 0; i < n && pos < dec_len; i++) {
            uint8_t l = p->payload[pos++];
            if (pos + l > dec_len) break;
            gatt_notify_msg(&p->payload[pos], l);
            pos += l;
        }
        return;
    }
    uint8_t *dest = &p->payload[enc_len];
    if (memcmp(dest, s_id, 4) == 0) {
        if (ptype == PAYLOAD_FRAG) {
            /* fix Bug 5: usar s_proto_out */
            uint16_t out_len;
            if (frag_recibir(emisor, p->payload, dec_len, s_proto_out, &out_len)) gatt_notify_msg(s_proto_out, out_len);
        } else {
            gatt_notify_msg(p->payload, dec_len);
        }
        ack_registrar(emisor, p->msg_id);
        ack_enviar_cum(emisor);
        return;
    }
    if (s_modo == MODE_ROOM_SERVER && ptype == PAYLOAD_TXT) room_guardar(dest, emisor, p->payload, dec_len);
    uint8_t route_type = p->header >> 6;
    uint8_t ttl = (p->header >> 4) & 0x03;
    if (s_modo == MODE_BRIDGE || ttl == 0) return;
    if (route_type == ROUTE_DIRECT && p->path_len >= 4) {
        if (memcmp(&p->path[0], s_id, 4) != 0) return;
        memmove(p->path, &p->path[4], p->path_len - 4);
        p->path_len -= 4;
        p->header = (p->header & 0xCF) | ((ttl-1) << 4);
        congestion_marcar_tx(3088);
        ble_emitir(data, len, phy_decidir(rssi, 0.9f, p->path_len/4, false));
        return;
    }
    uint32_t delay = 20 + (esp_random() % 480);
    vTaskDelay(pdMS_TO_TICKS(delay));
    if (p->path_len + 4 <= PATH_MAX) {
        memcpy(&p->path[p->path_len], s_id, 4);
        p->path_len += 4;
    }
    p->header = (p->header & 0xCF) | ((ttl-1) << 4);
    congestion_marcar_tx(3088);
    ble_emitir(data, len, phy_decidir(rssi, (float)s_rx/(s_tx+1), p->path_len/4, false));
}
bool proto_enviar(uint8_t *dest, uint8_t *txt, uint8_t len) {
    if (!congestion_puede_enviar()) return false;
    if (len <= BATCH_LEN && dest) return batch_add(dest, txt, len);
    memcpy(s_frag_dest, dest, 4);
    return frag_enviar(dest, txt, len, frag_cb);
    vecino_t *next = dest ? elegir_salto(dest) : NULL;
    packet_t p = {0};
    uint8_t route = next ? ROUTE_DIRECT : ROUTE_FLOOD;
    p.header = (route << 6) | (PAYLOAD_TXT << 2) | ((TTL_MAX-1) << 4) | 0x01;
    p.msg_id = ack_proxima_seq();
    if (next) {
        p.path_len = 4;
        memcpy(p.path, next->id, 4);
        reg_envio(next->id);
    } else {
        p.path_len = 4;
        memcpy(p.path, s_id, 4);
    }
    uint8_t buf[PAYLOAD_MAX];
    memcpy(buf, txt, len);
    int enc = crypto_encrypt(buf, len, p.nonce);
    if (enc < 0) return false;
    p.payload_len = enc + 4;
    memcpy(p.payload, buf, enc);
    uint8_t d[4] = {0xFF,0xFF,0xFF,0xFF};
    memcpy(&p.payload[enc], dest ? dest : d, 4);
    s_tx++;
    congestion_marcar_tx(3088);
    return ble_emitir((uint8_t*)&p, sizeof(p), phy_decidir(-50, 1.0f, 0, false));
}
void proto_enviar_beacon(void) {
    packet_t p = {0};
    p.header = (ROUTE_FLOOD << 6) | (PAYLOAD_ADVERT << 2) | ((TTL_MAX-1) << 4) | 0x01;
    p.msg_id = (uint8_t)esp_random();
    p.path_len = 4;
    memcpy(p.path, s_id, 4);
    uint8_t bat[1] = { power_bat_pct() };
    int enc = crypto_encrypt(bat, 1, p.nonce);
    p.payload_len = enc + 4;
    memcpy(p.payload, bat, enc);
    memcpy(&p.payload[enc], s_id, 4);
    congestion_marcar_tx(3088);
    ble_emitir((uint8_t*)&p, sizeof(p), phy_actual());
}
void proto_actualizar(void) {
    uint32_t ahora = xTaskGetTickCount();
    for (int i = 0; i < MAX_VECINOS; i++)
        if (s_vecinos[i].activo && (ahora - s_vecinos[i].visto) > pdMS_TO_TICKS(30000))
            s_vecinos[i].activo = false;
    frag_expirar(); room_expirar(); ack_expirar(); batch_expirar(); recalcular();
}
