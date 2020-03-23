#pragma once

#include <esp_types.h>
#include "esp_err.h"
#include <string.h>
#include <esp_log.h>

#define PID_NAME_MAX_LENGTH 16

typedef struct pid_handle_impl {
    float output;
} *pid_handle_t;

pid_handle_t pid_init(const char* name, float instant_gain, float integral_gain, float derivative_gain);

esp_err_t pid_update(pid_handle_t handle, float target, float current);

esp_err_t pid_reset(pid_handle_t handle);

esp_err_t pid_set_gains(pid_handle_t handle, float instant_gain, float integral_gain, float derivative_gain);

esp_err_t pid_terminate(pid_handle_t handle);