idf_component_register(
    SRCS "app_main.c" "power_manager.c" "ble_manager.c" "gatt_server.c" "protocol.c" "phy_manager.c" "crypto.c" "dedup.c" "fragment.c" "room.c" "ack_manager.c" "batch.c" "bridge.c" "congestion.c" "id_rotation.c" "uart_config.c" "ota_manager.c"
    INCLUDE_DIRS "."
    REQUIRES bt esp_adc nvs_flash mbedtls esp_driver_gpio esp_event esp_netif app_update esp_http_server esp_wifi)
