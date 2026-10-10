#include "config.h"
#include "power_manager.h"
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
    ESP_LOGI(TAG, "=== Xiaomesh v%s (Fase 1: Esqueleto) ===", FW_VERSION);
    led_init();
    led_patron(0); // Patrón de arranque

    power_init();

    while (1) {
        uint8_t bat = power_bat_pct();
        uint32_t panel = power_panel_mv();
        
        ESP_LOGI(TAG, "Batería: %d%% | Panel: %lu mV", bat, panel);

        if (bat < BATERIA_BAJA) {
            led_patron(4); // Patrón de batería baja
        } else {
            led_patron(2); // Patrón de funcionamiento normal
        }

        // Ejemplo de deep sleep si la batería es crítica y no hay sol
        if (bat < 15 && panel < 500) {
            ESP_LOGW(TAG, "Batería crítica. Entrando en deep sleep.");
            power_sleep(60);
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Esperar 5 segundos
    }
}
