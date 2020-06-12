#include "flight_control.h"

typedef struct {
    float curve;
} axis_curve_handle_impl;

axis_curve_handle_t axis_curve_init(float curve) {
    axis_curve_handle_impl* handle_impl = (axis_curve_handle_impl*)malloc(sizeof(axis_curve_handle_impl));

    handle_impl->curve = curve;

    return (axis_curve_handle_t) handle_impl;
}

float axis_curve_calculate(axis_curve_handle_t handle, float input) {
    axis_curve_handle_impl* handle_impl = (axis_curve_handle_impl*) handle;

    // 1-curve X + curve X^3
    return (1 - handle_impl->curve) * input + handle_impl->curve * (input * input * input);
}

