#pragma once

#include "axis_pid.h"
#include "gyro_control.h"
#include "ibus_duplex.h"
#include "servo_control.h"
#include "dshot_esc.h"
#include "math.h"
#include "axis_curve.h"
#include "battery_meter.h"

// #define PANDION_GYRO_ENABLED

typedef enum {
    RWTILT_CHAN = 0,
    LWTILT_CHAN,
    RWTRANS_CHAN,
    LWTRANS_CHAN,
    ELEVATOR_CHAN,
    RUDDER_CHAN,
    SERVO_CHAN_COUNT
} servo_ctrl_channel_t;

typedef enum {
    TRANS_VERTICAL,
    TRANS_MID,
    TRANS_HORIZONTAL,
    TRANS_UNSET
} transition_state_t;

esp_err_t flight_control_init();

esp_err_t flight_control_update();
