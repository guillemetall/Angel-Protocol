idf_component_register(
    SRCS "app_main.c" "power_manager.c"
    INCLUDE_DIRS "."
    REQUIRES esp_adc esp_driver_gpio
)
