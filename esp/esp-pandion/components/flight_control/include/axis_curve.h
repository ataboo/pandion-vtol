#pragma once

#include "esp_log.h"

typedef struct axis_curve_handle_impl *axis_curve_handle_t;

axis_curve_handle_t axis_curve_init(float curve);

float axis_curve_calculate(axis_curve_handle_t handle, float input);
