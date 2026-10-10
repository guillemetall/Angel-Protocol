#include "room.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static struct { uint8_t dest[4]; uint8_t src[4]; uint8_t data[ROOM_MSG_LEN]; uint8_t len; uint32_t ts; bool usado; } s_room[ROOM_MAX];
void room_init(void) { memset(s_room, 0, sizeof(s_room)); }
bool room_guardar(uint8_t *dest, uint8_t *src, uint8_t *data, uint8_t len) {
    if (len > ROOM_MSG_LEN) return false;
    for (int i = 0; i < ROOM_MAX; i++)
        if (!s_room[i].usado) {
            memcpy(s_room[i].dest, dest, 4);
            memcpy(s_room[i].src, src, 4);
            memcpy(s_room[i].data, data, len);
            s_room[i].len = len; s_room[i].ts = xTaskGetTickCount(); s_room[i].usado = true;
            return true;
        }
    return false;
}
int room_recoger(uint8_t *dest, uint8_t *out, int max) {
    int n = 0;
    for (int i = 0; i < ROOM_MAX && n < max; i++)
        if (s_room[i].usado && memcmp(s_room[i].dest, dest, 4) == 0) {
            memcpy(out + n * (4 + 4 + 1 + ROOM_MSG_LEN), &s_room[i], sizeof(s_room[i]));
            s_room[i].usado = false; n++;
        }
    return n;
}
void room_expirar(void) {
    uint32_t ahora = xTaskGetTickCount();
    for (int i = 0; i < ROOM_MAX; i++)
        if (s_room[i].usado && (ahora - s_room[i].ts) > pdMS_TO_TICKS(72*3600*1000))
            s_room[i].usado = false;
}
int room_count(void) { int n = 0; for (int i = 0; i < ROOM_MAX; i++) if (s_room[i].usado) n++; return n; }
