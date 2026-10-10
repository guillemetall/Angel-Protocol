/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "ble_manager.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include <string.h>
#define TAG "BLE"
#define SVC_UUID 0x00FF
#define CHR_MSG  0xFF01
static esp_gatt_if_t s_gatts_if = 0;
static uint16_t s_conn_id = 0xFFFF;
static uint16_t s_h_svc = 0;
static uint16_t s_h_msg = 0;
static bool s_adv_started = false;
typedef void (*ble_msg_cb_t)(uint8_t *data, uint16_t len);
static ble_msg_cb_t s_msg_cb = NULL;
static void start_adv(void) {
    if (s_adv_started) return;
    esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    esp_ble_gap_start_advertising(&adv_params);
    s_adv_started = true;
}
static void gap_cb(esp_gap_ble_cb_event_t e, esp_ble_gap_cb_param_t *p) {
    switch (e) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            start_adv();
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (p->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Advertising iniciado");
            }
            break;
        default: break;
    }
}
static void gatts_cb(esp_gatts_cb_event_t e, esp_gatt_if_t iface, esp_ble_gatts_cb_param_t *p) {
    switch (e) {
        case ESP_GATTS_REG_EVT:
            s_gatts_if = iface;
            esp_ble_gap_set_device_name("XIAO-MESH");
            esp_ble_gatts_create_service(iface, &(esp_gatt_srvc_id_t){
                .is_primary = true,
                .id = { .inst_id = 0, .uuid = { .len = 2, .uuid = { SVC_UUID } } }
            }, 4);
            break;
        case ESP_GATTS_CREATE_EVT:
            s_h_svc = p->create.service_handle;
            esp_ble_gatts_start_service(s_h_svc);
            esp_ble_gatts_add_char(s_h_svc, &(esp_bt_uuid_t){.len=2,.uuid={CHR_MSG}},
                ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                NULL, NULL);
            break;
        case ESP_GATTS_ADD_CHAR_EVT:
            if (p->add_char.char_uuid.uuid.uuid16 == CHR_MSG)
                s_h_msg = p->add_char.attr_handle;
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(TAG, "Servicio GATT iniciado");
            break;
        case ESP_GATTS_CONNECT_EVT:
            s_conn_id = p->connect.conn_id;
            ESP_LOGI(TAG, "Cliente conectado");
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            s_conn_id = 0xFFFF;
            s_adv_started = false;
            ESP_LOGI(TAG, "Cliente desconectado");
            start_adv();
            break;
        case ESP_GATTS_WRITE_EVT:
            if (p->write.handle == s_h_msg && p->write.len > 0) {
                ESP_LOGI(TAG, "Recibido: %.*s", p->write.len, p->write.value);
                if (s_msg_cb) s_msg_cb(p->write.value, p->write.len);
                if (s_conn_id != 0xFFFF) {
                    esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_h_msg,
                        p->write.len, p->write.value, false);
                }
            }
            break;
        default: break;
    }
}
void ble_init(void) {
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_cb));
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_cb));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = false,
        .min_interval = 0x20,
        .max_interval = 0x40,
        .appearance = 0x00,
        .manufacturer_len = 0, .p_manufacturer_data = NULL,
        .service_data_len = 0, .p_service_data = NULL,
        .service_uuid_len = 2,
        .p_service_uuid = (uint8_t[]){ 0xFF, 0x00 },
        .flag = 0x06,
    };
    esp_ble_gap_config_adv_data(&adv_data);
}
void ble_send(uint8_t *data, uint16_t len) {
    if (s_conn_id == 0xFFFF) return;
    esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_h_msg, len, data, false);
}
void ble_set_msg_cb(ble_msg_cb_t cb) { s_msg_cb = cb; }
