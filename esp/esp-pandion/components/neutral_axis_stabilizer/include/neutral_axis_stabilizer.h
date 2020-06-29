#pragma once

#include "axis_pid.h"
#include <esp_err.h>
#include "flight_control_common.h"
#include "gyro_control.h"

esp_err_t neutral_axis_stabilizer_init();

esp_err_t neutral_axis_stabilizer_update(transition_state_t transition_state, axis_duties_t* input_axes, gyro_values_t* gyro_values);

void neutral_axis_stabilizer_reset();
