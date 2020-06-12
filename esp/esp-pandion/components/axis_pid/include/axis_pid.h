#pragma once

#include <esp_types.h>
#include "esp_err.h"
#include <string.h>
#include <esp_log.h>

#define PID_NAME_MAX_LENGTH 16

typedef struct pid_handle_impl {
    float output;
} *pid_handle_t;

typedef struct {
    float k_p;
    float k_i;
    float k_d;
} pid_constants_t;

pid_handle_t pid_init(const char* name, pid_constants_t* pid_constants);

esp_err_t pid_update(pid_handle_t handle, float target, float current);

esp_err_t pid_reset(pid_handle_t handle);

esp_err_t pid_set_gains(pid_handle_t handle, pid_constants_t* pid_constants);

esp_err_t pid_terminate(pid_handle_t handle);

axis_curve_handle_t axis_curve_init(float curve);

float axis_curve_calculate(axis_curve_handle_t handle, float input);