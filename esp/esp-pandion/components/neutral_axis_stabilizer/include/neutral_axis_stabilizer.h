#pragma once

#include "axis_pid.h"
#include <esp_err.h>
#include "flight_control.h"


typedef struct {
    float pitch;
    float roll;
    float yaw;
    float throttle;
} stabilizer_input_t;

esp_err_t neutral_axis_stabilizer_init();


esp_err_t neutral_axis_stabilizer_update(stabilizer_input_t input);
