/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "bridge.h"
#include "gatt_server.h"
#include "protocol.h"
#include "esp_random.h"
#include <string.h>

#define BRIDGE_ACK_RECIBIDO  0x01
#define BRIDGE_ACK_ENTREGADO 0x02

static bool s_on = false;

void bridge_init(void) { s_on = false; }
void bridge_set_activo(bool on) { s_on = on; }
bool bridge_activo(void) { return s_on; }

void bridge_iphone_rx(uint8_t *data, uint16_t len) {
    if (!s_on || len < 5) return;

    uint8_t dest[4];
    memcpy(dest, &data[len-4], 4);

    uint8_t msg_id = (uint8_t)(esp_random() & 0xFF);

    proto_enviar(dest, data, len-4);

    uint8_t ack[2] = { BRIDGE_ACK_RECIBIDO, msg_id };
    gatt_notify_msg(ack, 2);
}

void bridge_mesh_rx(uint8_t *data, uint16_t len) {
    if (!s_on) return;
    gatt_notify_msg(data, len);
}
