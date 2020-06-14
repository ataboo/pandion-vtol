#include "flight_control_common.h"

float clampf(float value, float lower, float upper) {
    return fmin(upper, fmax(lower, value));
}