/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "config.h"
#include "power_manager.h"
#include "ble_manager.h"
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
static void on_msg(uint8_t *data, uint16_t len) {
    ESP_LOGI(TAG, "Mensaje del telefono: %.*s", len, data);
}
void app_main(void) {
    ESP_LOGI(TAG, "=== Xiaomesh v%s (Fase 2: BLE) ===", FW_VERSION);
    led_init();
    led_patron(0);
    power_init();
    ble_init();
    ble_set_msg_cb(on_msg);
    led_patron(1);
    while (1) {
        uint8_t bat = power_bat_pct();
        uint32_t panel = power_panel_mv();
        ESP_LOGI(TAG, "Bateria: %d%% | Panel: %lu mV", bat, panel);
        led_patron((bat < BATERIA_BAJA) ? 4 : 2);
        if (bat < 15 && panel < 500) {
            ESP_LOGW(TAG, "Bateria critica. Deep sleep.");
            power_sleep(60);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
