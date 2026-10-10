#include "ota_manager.h"
#include "config.h"
#include "esp_ota_ops.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "mbedtls/sha256.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static esp_ota_handle_t s_ota = 0;
static const esp_partition_t *s_part = NULL;
static bool s_ota_activo = false;
static uint8_t s_sha[32];
static bool s_sha_set = false;
bool ota_verificar_sha256(void) {
    if (!s_sha_set) return true;
    const esp_partition_t *part = esp_ota_get_next_update_partition(NULL);
    uint8_t hash[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    uint8_t buf[256];
    for (size_t off = 0; off < part->size; off += sizeof(buf)) {
        size_t c = (part->size - off > sizeof(buf)) ? sizeof(buf) : (part->size - off);
        esp_partition_read(part, off, buf, c);
        mbedtls_sha256_update(&ctx, buf, c);
    }
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    return memcmp(hash, s_sha, 32) == 0;
}
void ota_set_sha256(const uint8_t *sha) { memcpy(s_sha, sha, 32); s_sha_set = true; }
void ota_handle_write(uint8_t *data, uint16_t len) {
    if (len < 1) return;
    uint8_t cmd = data[0];
    if (cmd == 0x01) {
        s_part = esp_ota_get_next_update_partition(NULL);
        if (!s_part) return;
        esp_ota_begin(s_part, OTA_SIZE_UNKNOWN, &s_ota);
        s_ota_activo = true;
    } else if (cmd == 0x02 && s_ota_activo) {
        if (esp_ota_write(s_ota, data+1, len-1) != ESP_OK) { esp_ota_abort(s_ota); s_ota_activo = false; }
    } else if (cmd == 0x03 && s_ota_activo) {
        if (esp_ota_end(s_ota) == ESP_OK && ota_verificar_sha256())
            if (esp_ota_set_boot_partition(s_part) == ESP_OK) {
                vTaskDelay(pdMS_TO_TICKS(500));
                esp_restart();
            }
        s_ota_activo = false;
    } else if (cmd == 0x04) {
        if (s_ota_activo) esp_ota_abort(s_ota);
        s_ota_activo = false;
    } else if (cmd == 0x05 && len >= 33) {
        ota_set_sha256(&data[1]);
    }
}
static esp_err_t http_ota_post(httpd_req_t *req) {
    char buf[1024];
    int rem = req->content_len;
    const esp_partition_t *part = esp_ota_get_next_update_partition(NULL);
    esp_ota_handle_t h;
    if (esp_ota_begin(part, rem, &h) != ESP_OK) return ESP_FAIL;
    while (rem > 0) {
        int ret = httpd_req_recv(req, buf, sizeof(buf) < rem ? sizeof(buf) : rem);
        if (ret <= 0) { esp_ota_abort(h); return ESP_FAIL; }
        esp_ota_write(h, buf, ret);
        rem -= ret;
    }
    if (esp_ota_end(h) == ESP_OK && ota_verificar_sha256())
        if (esp_ota_set_boot_partition(part) == ESP_OK) {
            httpd_resp_send(req, "OK", 2);
            vTaskDelay(pdMS_TO_TICKS(500));
            esp_restart();
        }
    httpd_resp_send(req, "FAIL", 4);
    return ESP_FAIL;
}
static const httpd_uri_t s_ota_uri = { .uri = "/ota", .method = HTTP_POST, .handler = http_ota_post };
void ota_wifi_start(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    wifi_config_t ap = { .ap = { .ssid = "XIAO-MESH-OTA", .password = "mesh1234",
        .max_connection = 2, .authmode = WIFI_AUTH_WPA2_PSK } };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap);
    esp_wifi_start();
    httpd_config_t hc = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t srv;
    httpd_start(&srv, &hc);
    httpd_register_uri_handler(srv, &s_ota_uri);
}
void ota_modo_recuperacion(void) {
    gpio_config_t io = { .pin_bit_mask = (1ULL << RECOVERY_GPIO), .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE };
    gpio_config(&io);
    if (gpio_get_level(RECOVERY_GPIO) == 0) {
        ota_wifi_start();
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
