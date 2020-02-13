#pragma once

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"


struct motor_control_cfg {
    int gpio_a;
    int gpio_b;
    mcpwm_unit_t unit;
    mcpwm_timer_t timer;
    float duty_cycle;
};

typedef struct motor_control_impl *motor_control_handle_t;

motor_control_handle_t motor_control_init(struct motor_control_cfg *config);

void motor_control_move_up(motor_control_handle_t motor_handle);

void motor_control_move_down(motor_control_handle_t motor_handle);

void motor_control_stop(motor_control_handle_t motor_handle);