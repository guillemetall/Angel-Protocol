#include "gatt_server.h"
#include "config.h"
#include "protocol.h"
#include "phy_manager.h"
#include "ota_manager.h"
#include "crypto.h"
#include "bridge.h"
#include "ble_manager.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
static uint16_t s_conns[MAX_CONN];
static uint8_t s_conn_count = 0;
static uint16_t s_h_svc, s_h_cfg, s_h_msg, s_h_ota, s_h_status;
static esp_gatt_if_t s_gatts_if = 0;
static void nvs_guarda(uint8_t cmd, uint8_t val) {
    nvs_handle_t h;
    if (nvs_open("mesh", NVS_READWRITE, &h) == ESP_OK) {
        char key[8]; snprintf(key, sizeof(key), "cfg_%d", cmd);
        nvs_set_u8(h, key, val); nvs_commit(h); nvs_close(h);
    }
}
static uint8_t nvs_leer(uint8_t cmd, uint8_t def) {
    nvs_handle_t h; uint8_t v = def;
    if (nvs_open("mesh", NVS_READONLY, &h) == ESP_OK) {
        char key[8]; snprintf(key, sizeof(key), "cfg_%d", cmd);
        nvs_get_u8(h, key, &v); nvs_close(h);
    }
    return v;
}
void gatt_cargar_cfg(void) {
    phy_set_modo((phy_mode_t)nvs_leer(0x01, PHY_AUTO));
    proto_set_modo_nodo((node_mode_t)nvs_leer(0x02, MODE_REPEATER));
}
static void gatts_cb(esp_gatts_cb_event_t e, esp_gatt_if_t iface, esp_ble_gatts_cb_param_t *p) {
    switch (e) {
        case ESP_GATTS_REG_EVT:
            s_gatts_if = iface;
            esp_ble_gap_set_device_name("XIAO-MESH");
            esp_ble_gatts_create_service(iface, &(esp_gatt_srvc_id_t){
                .is_primary = true,
                .id = { .inst_id = 0, .uuid = { .len = 2, .uuid = { SVC_UUID } } }
            }, 8);
            break;
        case ESP_GATTS_CREATE_EVT:
            s_h_svc = p->create.service_handle;
            esp_ble_gatts_start_service(s_h_svc);
            esp_ble_gatts_add_char(s_h_svc, &(esp_bt_uuid_t){.len=2,.uuid={CHR_CFG}},
                ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_WRITE, NULL, NULL);
            esp_ble_gatts_add_char(s_h_svc, &(esp_bt_uuid_t){.len=2,.uuid={CHR_MSG}},
                ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_NOTIFY, NULL, NULL);
            esp_ble_gatts_add_char(s_h_svc, &(esp_bt_uuid_t){.len=2,.uuid={CHR_OTA}},
                ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_NOTIFY, NULL, NULL);
            esp_ble_gatts_add_char(s_h_svc, &(esp_bt_uuid_t){.len=2,.uuid={CHR_STATUS}},
                ESP_GATT_PERM_READ,
                ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_NOTIFY, NULL, NULL);
            break;
        case ESP_GATTS_ADD_CHAR_EVT: {
            uint16_t u = p->add_char.char_uuid.uuid.uuid16;
            if (u == CHR_CFG) s_h_cfg = p->add_char.attr_handle;
            else if (u == CHR_MSG) s_h_msg = p->add_char.attr_handle;
            else if (u == CHR_OTA) s_h_ota = p->add_char.attr_handle;
            else if (u == CHR_STATUS) s_h_status = p->add_char.attr_handle;
            break;
        }
        case ESP_GATTS_CONNECT_EVT:
            if (s_conn_count < MAX_CONN) {
                s_conns[s_conn_count++] = p->connect.conn_id;
                bridge_set_activo(true);
            }
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            for (int i = 0; i < s_conn_count; i++)
                if (s_conns[i] == p->disconnect.conn_id) { s_conns[i] = s_conns[--s_conn_count]; break; }
            if (s_conn_count == 0) bridge_set_activo(false);
            break;
        case ESP_GATTS_WRITE_EVT: {
            uint16_t h = p->write.handle;
            uint8_t *v = p->write.value;
            if (h == s_h_cfg && p->write.len >= 2) {
                switch (v[0]) {
                    case 0x01: phy_set_modo((phy_mode_t)v[1]); nvs_guarda(0x01, v[1]); break;
                    case 0x02: proto_set_modo_nodo((node_mode_t)v[1]); nvs_guarda(0x02, v[1]); break;
                    case 0x03: esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, v[1]); nvs_guarda(0x03, v[1]); break;
                    case 0x04: if (p->write.len >= 2 + AES_KEY_LEN) crypto_set_key(&v[2]); break;
                }
            } else if (h == s_h_msg && p->write.len >= 5) {
                bridge_iphone_rx(v, p->write.len);
            } else if (h == s_h_ota && p->write.len > 0) {
                ota_handle_write(v, p->write.len);
            }
            break;
        }
        default: break;
    }
}
void gatt_server_init(void) {
    nvs_flash_init();
    esp_ble_gatts_register_callback(gatts_cb);
    esp_ble_gatts_app_register(0);
    gatt_cargar_cfg();
}
void gatt_notify_status(uint8_t *data, uint16_t len) {
    for (int i = 0; i < s_conn_count; i++)
        esp_ble_gatts_send_indicate(s_gatts_if, s_conns[i], s_h_status, len, data, false);
}
void gatt_notify_msg(uint8_t *data, uint16_t len) {
    for (int i = 0; i < s_conn_count; i++)
        esp_ble_gatts_send_indicate(s_gatts_if, s_conns[i], s_h_msg, len, data, false);
}
