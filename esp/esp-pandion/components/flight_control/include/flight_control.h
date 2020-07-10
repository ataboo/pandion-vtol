#pragma once

// #define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "axis_curve.h"
#include "axis_pid.h"
#include "gyro_control.h"
#include "ibus_control.h"
#include "servo_control.h"
#include "dshot_esc.h"
#include "math.h"
#include "battery_meter.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "neutral_axis_stabilizer.h"
#include "positive_axis_stabilizer.h"
#include "flight_control_common.h"
#include "tcp_server.h"
#include "config_db.h"

#define PANDION_GYRO_ENABLED

typedef enum {
    RWTILT_CHAN = 0,
    LWTILT_CHAN,
    RWTRANS_CHAN,
    LWTRANS_CHAN,
    ELEVATOR_CHAN,
    RUDDER_CHAN,
    SERVO_CHAN_COUNT
} servo_ctrl_channel_t;

esp_err_t flight_control_init();

xQueueHandle init_timer();

esp_err_t pandion_server_commands_init();

esp_err_t pid_control_init();
