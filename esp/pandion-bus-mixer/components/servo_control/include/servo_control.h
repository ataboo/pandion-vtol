#pragma once

#include "driver/ledc.h"

typedef struct servo_ctrl_handle_impl *servo_ctrl_handle_t;

servo_ctrl_handle_t servo_ctrl_init();

esp_err_t servo_ctrl_set_channel(servo_ctrl_handle_t handle, ledc_channel_t servo_channel, uint32_t duty);

esp_err_t servo_ctrl_terminate(servo_ctrl_handle_t handle);
