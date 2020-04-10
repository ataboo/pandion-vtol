#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

// Voltage divider -- R1 690kOhn R2 220kOhm
#define BATTERY_GPIO_VOLTAGE_FACTOR 220.0/(220 + 690)

#define battery_vcc_to_gpio_int(vcc) ((int)(BATTERY_GPIO_VOLTAGE_FACTOR / 3.3 * 4095))

//12.9V
#define BATTERY_LEVEL_OVER battery_vcc_to_gpio_int(12.9)
//12.6V
#define BATTERY_LEVEL_FULL battery_vcc_to_gpio_int(12.6)
//10.0V => 2.75V
#define BATTERY_LEVEL_ALARM battery_vcc_to_gpio_int(10.0)
// 9.0V => 2.15V
#define BATTERY_LEVEL_EMPTY battery_vcc_to_gpio_int(9)

typedef enum {
    BATTERY_ERR,
    BATTERY_EMPTY,
    BATTERY_ALARM,
    BATTERY_NOMINAL,
    BATTERY_OVER,
} battery_state_t;

esp_err_t battery_meter_init();

float battery_meter_voltage();

battery_state_t battery_meter_state();

