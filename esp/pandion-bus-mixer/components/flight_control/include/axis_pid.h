#pragma once

#include <esp_types.h>
#include "esp_err.h"

#define PID_MEMORY_LENGTH 128

// CONFIG_PID_PK
// CONFIG_PID_PI
// CONFIG_PID_PD

typedef struct pid_handle_impl *pid_handle_t;

pid_handle_t pid_init();

esp_err_t pid_update(pid_handle_t handle, float target, float current, float *output);