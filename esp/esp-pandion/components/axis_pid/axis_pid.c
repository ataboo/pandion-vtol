#include "axis_pid.h"

typedef struct {
    float output;
    char name[PID_NAME_MAX_LENGTH];
    float instant_err;
    float integral_err;
    float derivative_err;
    pid_constants_t* pid_constants;
} pid_handle_impl;

// static const char* TAG = "AXIS_PID";

pid_handle_t pid_init(const char* name, pid_constants_t* pid_constants) {
    pid_handle_impl* handle = (pid_handle_impl*)malloc(sizeof(pid_handle_impl));

    strcpy(handle->name, name);
    handle->pid_constants = pid_constants;

    return (pid_handle_t)handle;
}

esp_err_t pid_update(pid_handle_t handle, float target, float current) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;
    
    handle_impl->derivative_err = target - current - handle_impl->instant_err;
    handle_impl->instant_err = target - current;
    handle_impl->integral_err += handle_impl->instant_err;

    handle_impl->output = handle_impl->instant_err * handle_impl->pid_constants->k_p +          
        handle_impl->integral_err * handle_impl->pid_constants->k_i +
        handle_impl->derivative_err * handle_impl->pid_constants->k_d;

    // ESP_LOGE("CONSOLE_PLOTTER", "[%f, %f, %f]", current, target, handle_impl->output);

    return ESP_OK;
}

esp_err_t pid_reset(pid_handle_t handle) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;
    
    handle_impl->instant_err = 0;
    handle_impl->integral_err = 0;
    handle_impl->derivative_err = 0;
    handle_impl->output = 0;

    return ESP_OK;
}

esp_err_t pid_set_gains(pid_handle_t handle, pid_constants_t* pid_constants) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;

    handle_impl->pid_constants = pid_constants;

    return ESP_OK;
}

esp_err_t pid_terminate(pid_handle_t handle) {
    pid_handle_impl* handle_impl = (pid_handle_impl*)handle;
    free(handle_impl);

    return ESP_OK;
}