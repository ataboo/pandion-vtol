#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

// Mode 2
#define IBUS_RX_CHAN_ROLL         0  // Right stick L/R
#define IBUS_RX_CHAN_PITCH        1  // Right stick U/D
#define IBUS_RX_CHAN_THROTTLE     2  // Left stick U/D
#define IBUS_RX_CHAN_RUDDER       3  // Left stick L/R
#define IBUS_RX_CHAN_TRANSITION   4  // R Shoulder 3-position
#define IBUS_RX_CHAN_ARM          5  // L Shoulder 2-position

typedef struct {
    // Should be 0x20, 0x40 (16416).
    uint16_t preamble;
    uint16_t channels[14];
    // 0xFF - sum of first 31 bytes.
    uint8_t chk_sum;
} ibus_rx_channel_vals_t;

typedef struct ibus_rx_handle_impl *ibus_rx_handle_t;

ibus_rx_handle_t ibus_rx_init();

void ibus_rx_terminate(ibus_rx_handle_t handle);

esp_err_t ibus_rx_update(ibus_rx_handle_t handle);

esp_err_t ibus_get_rx_channel_values(ibus_rx_handle_t ibus_handle, ibus_rx_channel_vals_t* channel_vals);
