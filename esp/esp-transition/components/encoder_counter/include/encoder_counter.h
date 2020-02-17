#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


typedef struct {
    gpio_num_t gpio_num;
} encoder_cfg_t;

typedef struct encoder_counter_impl *encoder_counter_handle_t;

encoder_counter_handle_t encoder_counter_init(encoder_cfg_t *config);

int encoder_counter_current_count(encoder_counter_handle_t handle);

void encoder_counter_terminate(encoder_counter_handle_t handle);

void encoder_counter_set_direction(encoder_counter_handle_t handle, int direction);
