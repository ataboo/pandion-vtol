#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "ibus_common.h"

// Mode 2
#define IBUS_RX_CHAN_ROLL         0  // Right stick L/R
#define IBUS_RX_CHAN_PITCH        1  // Right stick U/D
#define IBUS_RX_CHAN_THROTTLE     2  // Left stick U/D
#define IBUS_RX_CHAN_RUDDER       3  // Left stick L/R
#define IBUS_RX_CHAN_TRANSITION   4  // R Shoulder 3-position
#define IBUS_RX_CHAN_ARM          5  // L Shoulder 2-position

#define IBUS_CMD_CONTROL 0x40

typedef struct {
    uint16_t channels[14];
} ibus_ctrl_channel_vals_t;

esp_err_t ibus_control_update(ibus_ctrl_channel_vals_t* handle);
