#ifndef ROOM_H
#define ROOM_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void room_init(void);
bool room_guardar(uint8_t *dest, uint8_t *src, uint8_t *data, uint8_t len);
int room_recoger(uint8_t *dest, uint8_t *out, int max);
void room_expirar(void);
int room_count(void);
#endif
