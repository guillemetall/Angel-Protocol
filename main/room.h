#ifndef ROOM_H
#define ROOM_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    uint8_t  dest[4];
    uint8_t  src[4];
    uint8_t  data[ROOM_MSG_LEN];
    uint8_t  len;
    uint32_t ts;
    bool     usado;
} room_msg_t;

void room_init(void);
bool room_guardar(uint8_t *dest, uint8_t *src, uint8_t *data, uint8_t len);
int  room_recoger(uint8_t *dest, room_msg_t *out, int max);
int  room_entregar_pendientes(uint8_t *dest);
void room_expirar(void);
int  room_count(void);
#endif
