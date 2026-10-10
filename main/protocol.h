#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "config.h"
typedef struct __attribute__((packed)) {
    uint8_t header; uint8_t msg_id; uint8_t nonce[AES_NONCE_LEN];
    uint8_t path_len; uint8_t path[PATH_MAX];
    uint8_t payload_len; uint8_t payload[PAYLOAD_MAX + 4];
} packet_t;
typedef struct {
    uint8_t id[4]; int8_t rssi; float pdr; uint8_t bateria; uint8_t phy;
    uint8_t saltos_estimados; float score; uint16_t acks; uint16_t envios;
    uint32_t visto; bool activo;
} vecino_t;
void proto_init(void);
void proto_recibir(uint8_t *data, uint16_t len, int8_t rssi);
bool proto_enviar(uint8_t *dest, uint8_t *txt, uint8_t len);
void proto_actualizar(void);
void proto_set_modo(node_mode_t m);
uint8_t* proto_get_id(void);
int proto_get_vecinos(vecino_t **out);
void proto_enviar_beacon(void);
void proto_set_modo_phy(phy_mode_t m);
void proto_set_modo_nodo(node_mode_t m);
#endif
