#ifndef PHY_MANAGER_H
#define PHY_MANAGER_H
#include "config.h"
void phy_init(phy_mode_t modo);
void phy_set_modo(phy_mode_t m);
phy_mode_t phy_get_modo(void);
uint8_t phy_decidir(int8_t rssi, float pdr, uint8_t hops, bool sos);
void phy_actualizar(int8_t rssi, bool recibido);
uint8_t phy_actual(void);
#endif
