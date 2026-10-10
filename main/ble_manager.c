#include "ble_manager.h"
#include "config.h"
#include "protocol.h"
#include "gatt_server.h"
#include "security.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include <string.h>
#define TAG "BLE"
#define ADV_INST_GATT 1
#define ADV_INST_MESH 0
static bool s_gatt_adv_active = false;
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
        case ESP_GAP_BLE_SEC_REQ_EVT:
            if (security_pairing_ventana_abierta()) {
                ESP_LOGI(TAG, "Security request aceptada");
                esp_ble_gap_security_rsp(p->ble_security.ble_req.bd_addr, true);
            } else {
                ESP_LOGW(TAG, "Security request rechazada (ventana cerrada)");
                esp_ble_gap_security_rsp(p->ble_security.ble_req.bd_addr, false);
            }
            break;
        case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
            ESP_LOGW(TAG, ">>> PASSKEY: %06lu <<<", (unsigned long)p->ble_security.key_notif.passkey);
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            if (p->ble_security.auth_cmpl.success) {
                ESP_LOGI(TAG, "Emparejamiento OK");
                security_set_addr_auth(p->ble_security.auth_cmpl.bd_addr, true);
            } else {
                ESP_LOGW(TAG, "Emparejamiento fallido");
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
    gatt_server_init();
    esp_ble_gap_set_rand_addr((uint8_t[]){0xAA,0xBB,0xCC,0xDD,0xEE,0xFF});
    esp_ble_gap_ext_adv_set_params(ADV_INST_GATT, &s_adv_params_gatt);
    esp_ble_gap_ext_adv_set_params(ADV_INST_MESH, &s_adv_params_mesh);
    esp_ble_gap_set_ext_scan_params(&s_scan_params);
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint32_t passkey = security_get_passkey();
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
    ESP_LOGI(TAG, "BLE + SMP configurado (passkey: %06lu)", (unsigned long)passkey);
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
void ble_send(uint8_t *data, uint16_t len) { (void)data; (void)len; }
void ble_set_msg_cb(ble_msg_cb_t cb) { (void)cb; }
