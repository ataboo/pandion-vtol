#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

// $IDF_PATH/components/esptool_py/esptool/espefuse.py --port /dev/ttyUSB0 adc_info
#define BATTERY_LEVEL_VREF_MV 1121
#define BATTERY_LEVEL_MEAN_SAMPLE_COUNT 64
#define BATTERY_LEVEL_VREF_MV 1121
// Voltage divider -- R1 690kOhn R2 220kOhm (theoretical)
// #define BATTERY_GPIO_VOLTAGE_FACTOR 0.24175824175
// Measured
#define BATTERY_GPIO_VOLTAGE_FACTOR 0.2380952381
#define battery_vcc_to_gpio_int(vcc) ((int)(vcc * BATTERY_GPIO_VOLTAGE_FACTOR))
//12.9V
#define BATTERY_LEVEL_OVER_MV battery_vcc_to_gpio_int(12900)
//12.6V
#define BATTERY_LEVEL_FULL_MV battery_vcc_to_gpio_int(12600)
//10.0V
#define BATTERY_LEVEL_ALARM_MV battery_vcc_to_gpio_int(10000)
// 9.0V
#define BATTERY_LEVEL_EMPTY_MV battery_vcc_to_gpio_int(9000)

typedef enum {
    BATTERY_ERR,
    BATTERY_EMPTY,
    BATTERY_ALARM,
    BATTERY_NOMINAL,
    BATTERY_OVER,
} battery_state_t;

esp_err_t battery_meter_init();

uint32_t battery_meter_mv();

void battery_meter_update();

battery_state_t battery_meter_state();

