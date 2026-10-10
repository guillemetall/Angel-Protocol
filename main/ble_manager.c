#include "ble_manager.h"
#include "config.h"
#include "protocol.h"
#include "gatt_server.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include <string.h>
#define TAG "BLE"
#define SVC_UUID 0x00FF
#define CHR_MSG 0xFF01
#define ADV_INST_GATT 1
#define ADV_INST_MESH 0
static esp_gatt_if_t s_gatts_if = 0;
static uint16_t s_conn_id = 0xFFFF;
static uint16_t s_h_svc = 0;
static uint16_t s_h_msg = 0;
static bool s_mesh_adv_active = false;
static bool s_gatt_adv_active = false;
typedef void (*ble_msg_cb_t)(uint8_t *data, uint16_t len);
static ble_msg_cb_t s_msg_cb = NULL;
static esp_ble_gap_ext_adv_params_t s_adv_params_gatt = {
    .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_CONNECTABLE,
    .interval_min = 0x20, .interval_max = 0x40,
    .channel_map = ADV_CHNL_ALL,
    .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    .primary_phy = ESP_BLE_GAP_PHY_1M,
    .secondary_phy = ESP_BLE_GAP_PHY_1M,
    .own_addr_type = BLE_ADDR_TYPE_RANDOM,
    .sid = 1, .tx_power = 0,
};
static esp_ble_gap_ext_adv_params_t s_adv_params_mesh = {
    .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_NONCONN_NONSCANNABLE_UNDIRECTED,
    .interval_min = 0x20, .interval_max = 0x40,
    .channel_map = ADV_CHNL_ALL,
    .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    .primary_phy = ESP_BLE_GAP_PHY_CODED,
    .secondary_phy = ESP_BLE_GAP_PHY_CODED,
    .own_addr_type = BLE_ADDR_TYPE_RANDOM,
    .sid = 0, .tx_power = 0,
};
static esp_ble_ext_scan_params_t s_scan_params = {
    .own_addr_type = BLE_ADDR_TYPE_RANDOM,
    .filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
    .cfg_mask = ESP_BLE_GAP_EXT_SCAN_CFG_UNCODE_MASK | ESP_BLE_GAP_EXT_SCAN_CFG_CODE_MASK,
    .uncoded_cfg = { .scan_type = BLE_SCAN_TYPE_ACTIVE, .scan_interval = 0x50, .scan_window = 0x30 },
    .coded_cfg = { .scan_type = BLE_SCAN_TYPE_ACTIVE, .scan_interval = 0x50, .scan_window = 0x30 },
};
static void start_gatt_adv(void) {
    if (s_gatt_adv_active) return;
    esp_ble_gap_ext_adv_t adv = { .instance = ADV_INST_GATT, .duration = 0, .max_events = 0 };
    esp_ble_gap_ext_adv_start(1, &adv);
    s_gatt_adv_active = true;
}
static void start_mesh_adv(void) {
    if (s_mesh_adv_active) return;
    esp_ble_gap_ext_adv_t adv = { .instance = ADV_INST_MESH, .duration = 0, .max_events = 0 };
    esp_ble_gap_ext_adv_start(1, &adv);
    s_mesh_adv_active = true;
}
static void gap_cb(esp_gap_ble_cb_event_t e, esp_ble_gap_cb_param_t *p) {
    switch (e) {
        case ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT:
            if (p->ext_adv_set_params.status == ESP_BT_STATUS_SUCCESS) {
                uint8_t gatt_data[] = { 0x02, 0x01, 0x06, 0x03, 0x03, 0xFF, 0x00,
                    0x0A, 0x09, 'X','I','A','O','-','M','E','S','H' };
                esp_ble_gap_config_ext_adv_data_raw(ADV_INST_GATT, sizeof(gatt_data), gatt_data);
            }
            break;
        case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT:
            if (p->ext_adv_data_set.status == ESP_BT_STATUS_SUCCESS)
                start_gatt_adv();
            break;
        case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
            ESP_LOGI(TAG, "Ext Adv iniciado");
            break;
        case ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
            if (p->ext_adv_report.params.rssi != 0) {
                proto_recibir(p->ext_adv_report.params.adv_data,
                              p->ext_adv_report.params.adv_data_len,
                              p->ext_adv_report.params.rssi);
            }
            break;
        case ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT:
            esp_ble_gap_start_ext_scan(0, 0);
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
        case ESP_GATTS_CONNECT_EVT:
            s_conn_id = p->connect.conn_id;
            ESP_LOGI(TAG, "Cliente GATT conectado");
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            s_conn_id = 0xFFFF;
            s_gatt_adv_active = false;
            start_gatt_adv();
            break;
        case ESP_GATTS_WRITE_EVT:
            if (p->write.handle == s_h_msg && p->write.len > 0) {
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
    esp_ble_gap_set_rand_addr((uint8_t[]){0xAA,0xBB,0xCC,0xDD,0xEE,0xFF});
    esp_ble_gap_ext_adv_set_params(ADV_INST_GATT, &s_adv_params_gatt);
    esp_ble_gap_ext_adv_set_params(ADV_INST_MESH, &s_adv_params_mesh);
    esp_ble_gap_set_ext_scan_params(&s_scan_params);
}
bool ble_emitir(uint8_t *data, uint16_t len, uint8_t phy) {
    s_adv_params_mesh.secondary_phy = (phy == 3) ? ESP_BLE_GAP_PHY_CODED : ESP_BLE_GAP_PHY_1M;
    esp_ble_gap_ext_adv_set_params(ADV_INST_MESH, &s_adv_params_mesh);
    esp_ble_gap_config_ext_adv_data_raw(ADV_INST_MESH, len, data);
    esp_ble_gap_ext_adv_t adv = { .instance = ADV_INST_MESH, .duration = 0, .max_events = 0 };
    esp_ble_gap_ext_adv_start(1, &adv);
    return true;
}
void ble_scan_start(void) {
    esp_ble_gap_set_ext_scan_params(&s_scan_params);
}
void ble_send(uint8_t *data, uint16_t len) {
    if (s_conn_id == 0xFFFF) return;
    esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_h_msg, len, data, false);
}
void ble_set_msg_cb(ble_msg_cb_t cb) { s_msg_cb = cb; }
