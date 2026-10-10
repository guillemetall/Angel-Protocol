#ifndef LOW_PRIORITY_H
#define LOW_PRIORITY_H
#include <stdint.h>
#include <stdbool.h>
void low_priority_init(void);
bool low_priority_enqueue(uint8_t *data, uint16_t len);
void low_priority_tick(void);
bool low_priority_pending(void);
#endif
