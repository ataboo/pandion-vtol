#pragma once

#include <math.h>

typedef enum {
    TRANS_VERTICAL,
    TRANS_MID,
    TRANS_HORIZONTAL,
    TRANS_UNSET
} transition_state_t;

typedef struct {
    float roll;
    float pitch;
    float yaw;
    float throttle;
} axis_duties_t;

float clampf(float value, float lower, float upper);

