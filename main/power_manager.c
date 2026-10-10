#include "power_manager.h"
#include "config.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "esp_log.h"

static adc_oneshot_unit_handle_t s_adc;

void power_init(void) {
    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = ADC_UNIT_1 };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &s_adc));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, ADC_BAT_CH, &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, ADC_PANEL_CH, &chan_cfg));

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << WAKE_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_LOW_LEVEL,
    };
    gpio_config(&io_conf);
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable(WAKE_GPIO, GPIO_INTR_LOW_LEVEL);
    ESP_LOGI("POWER", "Gestor de energía inicializado.");
}

uint8_t power_bat_pct(void) {
    int raw = 0;
    adc_oneshot_read(s_adc, ADC_BAT_CH, &raw);
    float voltaje = (raw / 4095.0f) * 3.3f * 2.0f;
    if (voltaje >= 4.2f) return 100;
    if (voltaje <= 3.0f) return 0;
    return (uint8_t)((voltaje - 3.0f) / 1.2f * 100);
}

uint32_t power_panel_mv(void) {
    int raw = 0;
    adc_oneshot_read(s_adc, ADC_PANEL_CH, &raw);
    return (uint32_t)((raw / 4095.0f) * 3300 * 2.0f);
}

void power_sleep(uint32_t seg) {
    ESP_LOGI("POWER", "Entrando en deep sleep por %lu segundos...", seg);
    esp_sleep_enable_timer_wakeup((uint64_t)seg * 1000000);
    esp_deep_sleep_start();
}
