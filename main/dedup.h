#ifndef DEDUP_H
#define DEDUP_H
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
void dedup_init(void);
bool dedup_visto(uint32_t hash);
void dedup_marcar(uint32_t hash);
uint32_t dedup_hash_pkt(uint8_t *data, uint16_t len);
#endif
