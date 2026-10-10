#include "config.h"
#include "power_manager.h"
#include "ble_manager.h"
#include "phy_manager.h"
#include "protocol.h"
#include "gatt_server.h"
#include "crypto.h"
#include "congestion.h"
#include "ota_manager.h"
#include "uart_config.h"
#include "bridge.h"
#include "security.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
static const char *TAG = "APP";
static void led_init(void) {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}
static void led_patron(int modo) {
    int n = (modo == 0) ? 5 : (modo == 1) ? 1 : 2;
    int d = (modo == 0) ? 50 : 100;
    for (int i = 0; i < n; i++) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(d));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(d));
    }
}
void app_main(void) {
    led_init(); led_patron(0);
    ota_modo_recuperacion();
    security_init();
    power_init();
    crypto_init();
    bridge_init();
    ble_init();
    phy_init(PHY_AUTO);
    proto_init();
    ble_scan_start();
    uart_config_init();
    led_patron(1);
    uint32_t last_status = 0, last_beacon = 0;
    while (1) {
        security_tick();
        uart_config_tick();
        vTaskDelay(pdMS_TO_TICKS(200));
        uint32_t ahora = xTaskGetTickCount();
        if ((ahora - last_status) > pdMS_TO_TICKS(5000)) {
            last_status = ahora;
            proto_actualizar();
            uint8_t bat = power_bat_pct();
            uint32_t panel = power_panel_mv();
            uint8_t st[10] = { bat, phy_actual(), (uint8_t)(panel & 0xFF), (uint8_t)(panel >> 8), congestion_nivel(), 0,0,0,0,0 };
            gatt_notify_status(st, 10);
            ESP_LOGI(TAG, "Bat %d%% PHY %d", bat, phy_actual());
            if (bat < BATERIA_BAJA) led_patron(4);
            else if (phy_actual() == 3) led_patron(3);
            else led_patron(2);
            if (bat < 15 && panel < 500) power_sleep(60);
        }
        if ((ahora - last_beacon) > pdMS_TO_TICKS(BEACON_MS)) {
            last_beacon = ahora;
            proto_enviar_beacon();
        }
    }
}
