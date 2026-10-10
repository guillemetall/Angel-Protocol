#include "uart_config.h"
#include "protocol.h"
#include "phy_manager.h"
#include "driver/uart.h"
#include <string.h>
#define UART_N UART_NUM_0
void uart_config_init(void) {
    uart_config_t c = { .baud_rate = 115200, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE };
    uart_param_config(UART_N, &c);
    uart_driver_install(UART_N, 256, 0, 0, NULL, 0);
}
void uart_config_tick(void) {
    uint8_t buf[256];
    int len = uart_read_bytes(UART_N, buf, 255, 0);
    if (len <= 0) return;
    buf[len] = 0;
    if (strncmp((char*)buf, "set phy ", 8) == 0) {
        char *v = (char*)buf + 8;
        if (strncmp(v, "auto", 4) == 0) phy_set_modo(PHY_AUTO);
        else if (strncmp(v, "1m", 2) == 0) phy_set_modo(PHY_1M);
        else if (strncmp(v, "2m", 2) == 0) phy_set_modo(PHY_2M);
        else if (strncmp(v, "s8", 2) == 0) phy_set_modo(PHY_S8);
    } else if (strncmp((char*)buf, "set mode ", 9) == 0) {
        char *v = (char*)buf + 9;
        if (strncmp(v, "repeater", 8) == 0) proto_set_modo_nodo(MODE_REPEATER);
        else if (strncmp(v, "bridge", 6) == 0) proto_set_modo_nodo(MODE_BRIDGE);
        else if (strncmp(v, "room", 4) == 0) proto_set_modo_nodo(MODE_ROOM_SERVER);
    }
}
