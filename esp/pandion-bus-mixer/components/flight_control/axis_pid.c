#include "axis_pid.h"

struct pid_handle_impl {
    float mean_error;
    uint8_t err_sample_count;
};

pid_handle_t pid_init() {
    pid_handle_t handle = malloc(sizeof(pid_handle_impl));

    
}

esp_err_t pid_update(pid_handle_t handle, float target, float current, float *output) {
    float error = current - target;
    
    handle->mean_error = (handle->mean_error * handle->err_sample_count + error) / handle->err_sample_count + 1;
    handle->err_sample_count 

}