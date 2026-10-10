#include "power_manager.h"
#include "config.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
static adc_oneshot_unit_handle_t s_adc;
void power_init(void) {
    adc_oneshot_unit_init_cfg_t u = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&u, &s_adc);
    adc_oneshot_chan_cfg_t c = { .bitwidth = ADC_BITWIDTH_12, .atten = ADC_ATTEN_DB_12 };
    adc_oneshot_config_channel(s_adc, ADC_BAT_CH, &c);
    adc_oneshot_config_channel(s_adc, ADC_PANEL_CH, &c);
    gpio_config_t io = { .pin_bit_mask = (1ULL << WAKE_GPIO), .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE, .intr_type = GPIO_INTR_LOW_LEVEL };
    gpio_config(&io);
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable(WAKE_GPIO, GPIO_INTR_LOW_LEVEL);
}
uint8_t power_bat_pct(void) {
    int raw = 0;
    adc_oneshot_read(s_adc, ADC_BAT_CH, &raw);
    float v = (raw / 4095.0) * 3.3 * 2.0;
    if (v >= 4.2) return 100;
    if (v <= 3.0) return 0;
    return (uint8_t)((v - 3.0) / 1.2 * 100);
}
uint32_t power_panel_mv(void) {
    int raw = 0;
    adc_oneshot_read(s_adc, ADC_PANEL_CH, &raw);
    return (uint32_t)((raw / 4095.0) * 3300 * 2.0);
}
void power_sleep(uint32_t seg) {
    esp_sleep_enable_timer_wakeup((uint64_t)seg * 1000000);
    esp_deep_sleep_start();
}
