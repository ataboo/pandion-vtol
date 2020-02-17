#pragma once

#include "driver/uart.h"

typedef struct {
    // Should be 0x20, 0x40 (16416).
    uint16_t preamble;
    uint16_t channel_1;
    uint16_t channel_2;
    uint16_t channel_3;
    uint16_t channel_4;
    uint16_t channel_5;
    uint16_t channel_6;
    uint16_t channel_7;
    uint16_t channel_8;
    uint16_t channel_9;
    uint16_t channel_10;
    uint16_t channel_11;
    uint16_t channel_12;
    uint16_t channel_13;
    uint16_t channel_14;
    // 0xFF - sum of first 30 bytes.
    uint16_t chk_sum;
} ibus_channel_vals_t;

typedef struct ibus_duplex_handle_impl *ibus_duplex_handle_t;

ibus_duplex_handle_t ibus_duplex_init();

void ibus_duplex_terminate(ibus_duplex_handle_t handle);

uint16_t ibus_get_channel_value(ibus_duplex_handle_t ibus_handle);

esp_err_t ibus_get_channel_values(ibus_duplex_handle_t ibus_handle, ibus_channel_vals_t* channel_vals);
