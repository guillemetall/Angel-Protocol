#include "id_rotation.h"
#include "config.h"
#include "protocol.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static uint32_t s_ultimo = 0;
static bool s_pendiente = false;
void id_rot_init(uint8_t *id) { esp_fill_random(id, 4); s_ultimo = xTaskGetTickCount(); }
void id_rot_tick(void) {
    uint32_t ahora = xTaskGetTickCount();
    if ((ahora - s_ultimo) > pdMS_TO_TICKS(ID_ROTATE_MS)) s_pendiente = true;
}
bool id_rot_cambio(void) { return s_pendiente; }
void id_rot_aceptar(void) {
    uint8_t *id = proto_get_id();
    esp_fill_random(id, 4);
    s_pendiente = false;
    s_ultimo = xTaskGetTickCount();
}
