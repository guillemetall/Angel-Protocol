#include "phy_manager.h"
static phy_mode_t s_modo = PHY_AUTO;
static uint8_t s_phy = 1;
static int8_t s_rssi = -100;
static uint16_t s_cb = 0, s_ca = 0;
void phy_init(phy_mode_t m) { s_modo = m; s_phy = 1; }
void phy_set_modo(phy_mode_t m) { s_modo = m; }
phy_mode_t phy_get_modo(void) { return s_modo; }
uint8_t phy_decidir(int8_t rssi, float pdr, uint8_t hops, bool sos) {
    (void)rssi;
    if (s_modo == PHY_S8) return 3;
    if (s_modo == PHY_1M) return 1;
    if (s_modo == PHY_2M) return 2;
    if (sos) return 3;
    if (s_cb >= N_CONFIRMACION && pdr < PDR_MINIMO) return 3;
    if (hops >= 3) return 3;
    return 1;
}
void phy_actualizar(int8_t rssi, bool recibido) {
    (void)recibido;
    s_rssi = (int8_t)(0.3 * rssi + 0.7 * s_rssi);
    if (s_rssi < RSSI_S8) { s_cb++; s_ca = 0; }
    else if (s_rssi > RSSI_1M) { s_ca++; s_cb = 0; }
    if (s_ca >= N_CONFIRMACION && s_rssi > (RSSI_1M + HISTERESIS)) s_phy = 1;
    if (s_cb >= N_CONFIRMACION) s_phy = 3;
}
uint8_t phy_actual(void) { return s_phy; }
