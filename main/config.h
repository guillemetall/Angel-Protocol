#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include <stdbool.h>
#define FW_VERSION "5.0.0"
typedef enum { MODE_REPEATER, MODE_BRIDGE, MODE_ROOM_SERVER } node_mode_t;
typedef enum { PHY_AUTO, PHY_1M, PHY_2M, PHY_S8 } phy_mode_t;
typedef enum { ROUTE_FLOOD, ROUTE_DIRECT } route_type_t;
typedef enum { PAYLOAD_TXT, PAYLOAD_ACK, PAYLOAD_ADVERT, PAYLOAD_SOS, PAYLOAD_FRAG, PAYLOAD_ACK_CUM, PAYLOAD_BATCH } payload_type_t;
#define RSSI_S8 -83
#define RSSI_1M -75
#define HISTERESIS 8
#define N_CONFIRMACION 10
#define PDR_MINIMO 0.80f
#define MAX_VECINOS 16
#define MAX_RUTAS 8
#define TTL_MAX 7
#define PAYLOAD_MAX 20
#define PATH_MAX 64
#define FRAG_MAX 8
#define BATCH_MAX 4
#define BATCH_LEN 4
#define ROOM_MAX 32
#define ROOM_MSG_LEN 80
#define DEDUP_CACHE 32
#define MAX_CONN 8
#define LED_GPIO 15
#define ANT_SWITCH_GPIO 3
#define ANT_SELECT_GPIO 14
#define RECOVERY_GPIO 9
#define OTA_BUTTON_GPIO 9
#define OTA_WINDOW_MS 30000
#define ADC_BAT_CH 0
#define ADC_PANEL_CH 1
#define WAKE_GPIO 2
#define BATERIA_BAJA 20
#define SLEEP_MS 5000
#define BEACON_MS 10000
#define ID_ROTATE_MS 600000
#define CONGESTION_UMBRAL 60
#define CONGESTION_VENTANA 10000
#define SVC_UUID 0x00FF
#define CHR_CFG 0xFF01
#define CHR_MSG 0xFF02
#define CHR_OTA 0xFF03
#define CHR_STATUS 0xFF04
#define AES_KEY_LEN 16
#define AES_TAG_LEN 16
#define AES_NONCE_LEN 12
#define ADMIN_KEY_LEN 16
#define MAX_AUTH_SESSIONS 8
#endif
