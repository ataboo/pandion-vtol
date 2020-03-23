#include "axis_pid.h"

typedef struct {
    float output;
    char name[PID_NAME_MAX_LENGTH];
    float instant_gain;
    float integral_gain;
    float derivative_gain;
    float instant_err;
    float integral_err;
    float derivative_err;
} pid_handle_impl;

static const char* TAG = "AXIS_PID";

pid_handle_t pid_init(const char* name, float instant_gain, float integral_gain, float derivative_gain) {
    pid_handle_impl* handle = (pid_handle_impl*)malloc(sizeof(pid_handle_impl));

    strcpy(handle->name, name);
    handle->instant_gain = instant_gain;
    handle->integral_gain = integral_gain;
    handle->derivative_gain = derivative_gain;

    return (pid_handle_t)handle;
}

esp_err_t pid_update(pid_handle_t handle, float target, float current) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;
    
    handle_impl->derivative_err = target - current - handle_impl->instant_err;
    handle_impl->instant_err = target - current;
    handle_impl->integral_err += handle_impl->instant_err;

    handle_impl->output = handle_impl->instant_err * handle_impl->instant_gain +          
        handle_impl->integral_err * handle_impl->integral_gain +
        handle_impl->derivative_err * handle_impl->derivative_gain;

    // ESP_LOGE("CONSOLE_PLOTTER", "[%f, %f, %f]", current, target, handle_impl->output);


    return ESP_OK;
}

esp_err_t pid_reset(pid_handle_t handle) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;
    
    handle_impl->instant_err = 0;
    handle_impl->integral_err = 0;
    handle_impl->derivative_err = 0;

    return ESP_OK;
}

esp_err_t pid_set_gains(pid_handle_t handle, float instant_gain, float integral_gain, float derivative_gain) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;

    handle_impl->instant_gain = instant_gain;
    handle_impl->integral_gain = integral_gain;
    handle_impl->derivative_gain = derivative_gain;

    return ESP_OK;
}

esp_err_t pid_terminate(pid_handle_t handle) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;
    free(handle_impl);

    return ESP_OK;
}