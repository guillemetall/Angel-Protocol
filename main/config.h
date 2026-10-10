#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define FW_VERSION "2.0.0"

// Pines de hardware (según tu diseño)
#define LED_GPIO         15
#define ANT_SWITCH_GPIO  3
#define ANT_SELECT_GPIO  14
#define RECOVERY_GPIO    9
#define ADC_BAT_CH       0
#define ADC_PANEL_CH     1
#define WAKE_GPIO        2

// Configuración de energía
#define BATERIA_BAJA     20
#define SLEEP_MS         5000

#endif
