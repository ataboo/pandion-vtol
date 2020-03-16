#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

// Mode 2
#define IBUS_CHAN_ROLL         0  // Right stick L/R
#define IBUS_CHAN_PITCH        1  // Right stick U/D
#define IBUS_CHAN_THROTTLE     2  // Left stick U/D
#define IBUS_CHAN_RUDDER       3  // Left stick L/R
#define IBUS_CHAN_TRANSITION   4  // R Shoulder 3-position
#define IBUS_CHAN_ARM          5  // L Shoulder 2-position

typedef struct {
    // Should be 0x20, 0x40 (16416).
    uint16_t preamble;
    uint16_t channels[14];
    // 0xFF - sum of first 31 bytes.
    uint8_t chk_sum;
} ibus_channel_vals_t;

typedef struct ibus_duplex_handle_impl *ibus_duplex_handle_t;

ibus_duplex_handle_t ibus_duplex_init();

void ibus_duplex_terminate(ibus_duplex_handle_t handle);

esp_err_t ibus_duplex_update(ibus_duplex_handle_t handle);

uint16_t ibus_get_channel_value(ibus_duplex_handle_t ibus_handle, int value);

esp_err_t ibus_get_channel_values(ibus_duplex_handle_t ibus_handle, ibus_channel_vals_t* channel_vals);
