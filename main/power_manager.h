#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

void power_init(void);
uint8_t power_bat_pct(void);
uint32_t power_panel_mv(void);
void power_sleep(uint32_t seg);

#endif
