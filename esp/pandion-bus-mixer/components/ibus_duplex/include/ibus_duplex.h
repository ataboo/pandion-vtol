#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

typedef struct {
    // Should be 0x20, 0x40 (16416).
    uint16_t preamble;
    uint16_t channels[14];
    // 0xFF - sum of first 30 bytes.
    uint16_t chk_sum;
} ibus_channel_vals_t;

typedef struct ibus_duplex_handle_impl *ibus_duplex_handle_t;

ibus_duplex_handle_t ibus_duplex_init();

void ibus_duplex_terminate(ibus_duplex_handle_t handle);

uint16_t ibus_get_channel_value(ibus_duplex_handle_t ibus_handle, int value);

esp_err_t ibus_get_channel_values(ibus_duplex_handle_t ibus_handle, ibus_channel_vals_t* channel_vals);
