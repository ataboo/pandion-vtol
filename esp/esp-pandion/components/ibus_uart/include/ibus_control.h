#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "ibus_common.h"
#include <string.h>

// Mode 2
typedef enum {
    IBUS_RX_CHAN_ROLL = 0,    // Right stick L/R
    IBUS_RX_CHAN_PITCH,       // Right stick U/D
    IBUS_RX_CHAN_THROTTLE,    // Left stick U/D
    IBUS_RX_CHAN_RUDDER,      // Left stick L/R
    IBUS_RX_CHAN_ARM,         // L Shoulder 2-position
    IBUS_RX_CHAN_TRANSITION   // R Shoulder 3-position
} ibus_ctrl_channel_t;

#define IBUS_RX_CHAN_COUNT  14
#define IBUS_CMD_CONTROL    0x40

typedef struct {
    uint16_t channels[IBUS_RX_CHAN_COUNT];
} ibus_ctrl_channel_vals_t;

typedef struct ibus_ctrl_handle_impl *ibus_ctrl_handle_t;

ibus_ctrl_channel_vals_t* ibus_channel_vals_init();

ibus_ctrl_handle_t ibus_control_init(uart_port_t uart_num, gpio_num_t rx_gpio_pin);

esp_err_t ibus_control_update(ibus_ctrl_handle_t handle);

void ibus_control_channel_values(ibus_ctrl_handle_t handle, ibus_ctrl_channel_vals_t* vals);
