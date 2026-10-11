/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "bridge.h"
#include "gatt_server.h"
#include "protocol.h"
#include "esp_random.h"
#include <string.h>

#define BRIDGE_ACK_RECIBIDO  0x01
#define BRIDGE_ACK_ENTREGADO 0x02

static bool s_on = false;
static uint8_t s_ultimo_client_id = 0;
static bool s_esperando_ack = false;

void bridge_init(void) {
    s_on = false;
    s_esperando_ack = false;
}

void bridge_set_activo(bool on) { s_on = on; }
bool bridge_activo(void) { return s_on; }

void bridge_iphone_rx(uint8_t *data, uint16_t len) {
    if (!s_on || len < 5) return;

    uint8_t dest[4];
    memcpy(dest, &data[len-4], 4);

    uint8_t client_id = (uint8_t)(esp_random() & 0xFF);
    s_ultimo_client_id = client_id;
    s_esperando_ack = true;

    proto_enviar(dest, data, len-4);

    uint8_t ack[2] = { BRIDGE_ACK_RECIBIDO, client_id };
    gatt_notify_msg(ack, 2);
}

void bridge_mesh_rx(uint8_t *data, uint16_t len) {
    if (!s_on) return;
    gatt_notify_msg(data, len);
}

void bridge_notify_ack(void) {
    if (!s_on || !s_esperando_ack) return;
    uint8_t notif[2] = { BRIDGE_ACK_ENTREGADO, s_ultimo_client_id };
    gatt_notify_msg(notif, 2);
    s_esperando_ack = false;
}
