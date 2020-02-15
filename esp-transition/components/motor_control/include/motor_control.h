#pragma once

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "encoder_counter.h"


typedef struct {
    gpio_num_t gpio_a;
    gpio_num_t gpio_b;
    gpio_num_t gpio_encoder;
    mcpwm_unit_t unit;
    mcpwm_timer_t timer;
    float duty_cycle;
} motor_control_cfg_t;

typedef struct motor_control_impl *motor_control_handle_t;

motor_control_handle_t motor_control_init(motor_control_cfg_t *config);

void motor_control_move_fwd(motor_control_handle_t motor_handle);

void motor_control_move_back(motor_control_handle_t motor_handle);

void motor_control_stop(motor_control_handle_t motor_handle);

int motor_control_get_position(motor_control_handle_t motor_handle);
