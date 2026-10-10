#include "security.h"
#include "esp_log.h"
#include "esp_random.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#define TAG "SEC"
#define PASSKEY_DEFAULT   123456
#define PAIRING_WINDOW_MS 60000
#define BUTTON_HOLD_MS    3000
static uint32_t s_passkey = 0;
static uint8_t  s_admin_key[ADMIN_KEY_LEN];
static struct { uint8_t bd[6]; bool auth; } s_auth[MAX_AUTH_SESSIONS];
static uint32_t s_ota_ventana = 0;
static uint32_t s_pairing_ventana = 0;
static uint32_t s_boton_press = 0;
static bool s_boton_prev = true;
static void nvs_save_u32(const char *key, uint32_t v) {
    nvs_handle_t h;
    if (nvs_open("security", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u32(h, key, v); nvs_commit(h); nvs_close(h);
    }
}
static uint32_t nvs_load_u32(const char *key, uint32_t def) {
    nvs_handle_t h; uint32_t v = def;
    if (nvs_open("security", NVS_READONLY, &h) == ESP_OK) {
        nvs_get_u32(h, key, &v); nvs_close(h);
    }
    return v;
}
static void nvs_save_blob(const char *key, const void *data, size_t len) {
    nvs_handle_t h;
    if (nvs_open("security", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_blob(h, key, data, len); nvs_commit(h); nvs_close(h);
    }
}
static bool nvs_load_blob(const char *key, void *out, size_t len) {
    nvs_handle_t h; size_t sz = len; bool ok = false;
    if (nvs_open("security", NVS_READONLY, &h) == ESP_OK) {
        ok = (nvs_get_blob(h, key, out, &sz) == ESP_OK && sz == len);
        nvs_close(h);
    }
    return ok;
}
void security_init(void) {
    s_passkey = nvs_load_u32("passkey", 0);
    if (s_passkey == 0 || s_passkey > 999999) {
        s_passkey = PASSKEY_DEFAULT;
        nvs_save_u32("passkey", s_passkey);
    }
    ESP_LOGI(TAG, "Passkey: %06lu", (unsigned long)s_passkey);
    if (!nvs_load_blob("adminkey", s_admin_key, ADMIN_KEY_LEN)) {
        esp_fill_random(s_admin_key, ADMIN_KEY_LEN);
        nvs_save_blob("adminkey", s_admin_key, ADMIN_KEY_LEN);
        ESP_LOGW(TAG, ">>> ADMIN KEY generada <<<");
        ESP_LOG_BUFFER_HEX(TAG, s_admin_key, ADMIN_KEY_LEN);
    }
    memset(s_auth, 0, sizeof(s_auth));
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << OTA_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io);
}
uint32_t security_get_passkey(void) { return s_passkey; }
void security_set_passkey(uint32_t pk) {
    if (pk < 100000 || pk > 999999) return;
    s_passkey = pk;
    nvs_save_u32("passkey", pk);
    ESP_LOGW(TAG, "Passkey actualizado: %06lu", (unsigned long)pk);
}
void security_get_admin_key(uint8_t *out) { memcpy(out, s_admin_key, ADMIN_KEY_LEN); }
bool security_set_admin_key(const uint8_t *key) {
    memcpy(s_admin_key, key, ADMIN_KEY_LEN);
    nvs_save_blob("adminkey", s_admin_key, ADMIN_KEY_LEN);
    return true;
}
bool security_is_addr_auth(const uint8_t *bd_addr) {
    for (int i = 0; i < MAX_AUTH_SESSIONS; i++)
        if (memcmp(s_auth[i].bd, bd_addr, 6) == 0) return s_auth[i].auth;
    return false;
}
void security_set_addr_auth(const uint8_t *bd_addr, bool auth) {
    for (int i = 0; i < MAX_AUTH_SESSIONS; i++)
        if (memcmp(s_auth[i].bd, bd_addr, 6) == 0) { s_auth[i].auth = auth; return; }
    for (int i = 0; i < MAX_AUTH_SESSIONS; i++)
        if (s_auth[i].bd[0] == 0 && s_auth[i].bd[1] == 0) {
            memcpy(s_auth[i].bd, bd_addr, 6);
            s_auth[i].auth = auth;
            return;
        }
}
void security_remove_addr(const uint8_t *bd_addr) {
    for (int i = 0; i < MAX_AUTH_SESSIONS; i++)
        if (memcmp(s_auth[i].bd, bd_addr, 6) == 0) {
            memset(s_auth[i].bd, 0, 6);
            s_auth[i].auth = false;
            return;
        }
}
void security_ota_abrir_ventana(void) {
    s_ota_ventana = xTaskGetTickCount() + pdMS_TO_TICKS(OTA_WINDOW_MS);
    ESP_LOGW(TAG, ">>> Ventana OTA abierta por %d s <<<", OTA_WINDOW_MS/1000);
}
bool security_ota_ventana_abierta(void) {
    return (xTaskGetTickCount() < s_ota_ventana);
}
void security_abrir_pairing_ventana(void) {
    s_pairing_ventana = xTaskGetTickCount() + pdMS_TO_TICKS(PAIRING_WINDOW_MS);
    ESP_LOGW(TAG, ">>> Ventana de emparejamiento abierta por %d s <<<", PAIRING_WINDOW_MS/1000);
}
bool security_pairing_ventana_abierta(void) {
    return (xTaskGetTickCount() < s_pairing_ventana);
}
void security_tick(void) {
    bool actual = gpio_get_level(OTA_BUTTON_GPIO);
    if (s_boton_prev && !actual) {
        s_boton_press = xTaskGetTickCount();
    } else if (!s_boton_prev && actual) {
        uint32_t dur = xTaskGetTickCount() - s_boton_press;
        if (dur >= pdMS_TO_TICKS(BUTTON_HOLD_MS)) {
            security_abrir_pairing_ventana();
        } else if (dur >= pdMS_TO_TICKS(200)) {
            security_ota_abrir_ventana();
        }
    }
    s_boton_prev = actual;
}
