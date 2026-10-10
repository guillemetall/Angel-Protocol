#ifndef BATCH_H
#define BATCH_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void batch_init(void);
bool batch_add(uint8_t *dest, uint8_t *msg, uint8_t len);
void batch_flush(void);
void batch_expirar(void);
#endif
