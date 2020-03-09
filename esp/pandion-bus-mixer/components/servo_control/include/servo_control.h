#pragma once

#include "driver/ledc.h"
#include "esp_log.h"


typedef struct {
    uint32_t min_us;
    uint32_t max_us;
    gpio_num_t gpio_num;
} servo_ctrl_channel_cfg_t;

typedef struct servo_ctrl_handle_impl *servo_ctrl_handle_t;

servo_ctrl_handle_t servo_ctrl_init(servo_ctrl_channel_cfg_t* channel_cfgs, int channel_count);

esp_err_t servo_ctrl_set_channel_duty(servo_ctrl_handle_t handle, ledc_channel_t servo_channel, float duty);

esp_err_t servo_ctrl_terminate(servo_ctrl_handle_t handle);
