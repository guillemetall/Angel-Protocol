#include "bridge.h"
#include "gatt_server.h"
#include "protocol.h"
#include <string.h>
static bool s_on = false;
void bridge_init(void) { s_on = false; }
void bridge_set_activo(bool on) { s_on = on; }
bool bridge_activo(void) { return s_on; }
void bridge_iphone_rx(uint8_t *data, uint16_t len) {
    if (!s_on || len < 5) return;
    uint8_t dest[4];
    memcpy(dest, &data[len-4], 4);
    proto_enviar(dest, data, len-4);
}
void bridge_mesh_rx(uint8_t *data, uint16_t len) {
    if (!s_on) return;
    gatt_notify_msg(data, len);
}
