#include "battery_meter.h"                                  

static uint16_t level;

static void read_gpio_level(uint16_t* level) {
    *level = (uint16_t)gpio_get_level(CONFIG_BATTERY_VCC_GPIO);
}

esp_err_t battery_meter_init() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = CONFIG_BATTERY_VCC_GPIO,
        .pull_down_en = 0,
        .pull_up_en = 0
    };

    return gpio_config(&io_conf);
}

float battery_meter_voltage() {
    read_gpio_level(&level);

    return level / 4095.0 * 3.3 / BATTERY_GPIO_VOLTAGE_FACTOR;
}

battery_state_t battery_meter_state() {
    read_gpio_level(&level);
    
    if(level < BATTERY_LEVEL_EMPTY) {
        return BATTERY_EMPTY;
    }

    if (level < BATTERY_LEVEL_ALARM) {
        return BATTERY_ALARM;
    }

    if (level < BATTERY_LEVEL_FULL) {
        return BATTERY_NOMINAL;
    }

    return BATTERY_OVER;
}
