#include "positive_axis_stabilizer.h"

static pid_handle_t roll_pid_v;
static pid_handle_t pitch_pid_v;
static pid_handle_t yaw_pid_v;

// static pid_handle_t roll_pid_h;
// static pid_handle_t pitch_pid_h;
// static pid_handle_t yaw_pid_h;

static float max_roll_rads = PI / 4;
static float max_pitch_rads = PI / 4;
static float max_yaw_rate = 45.0;

static bool stabilizer_initialized = false;

esp_err_t positive_axis_stabilizer_init() {
    if (stabilizer_initialized) {
        return ESP_FAIL;
    }
    
    pid_constants_t roll_v_k = {0.002, 0.0005, 0.0025};
    pid_constants_t pitch_v_k = {0.04, 0.002, 0.0};
    pid_constants_t yaw_v_k = {0.002, 0.0001, 0.0};

    // pid_constants_t roll_h_k = {0.002, 0.0005, 0.000};
    // pid_constants_t pitch_h_k = {0.04, 0.002, 0.0};
    // pid_constants_t yaw_h_k = {0.002, 0.0005, 0.0025};

    roll_pid_v = pid_init("x_axis", &roll_v_k);
    pitch_pid_v = pid_init("y_axis", &pitch_v_k);
    yaw_pid_v = pid_init("z_axis", &yaw_v_k);

    // roll_pid_h = pid_init("x_axis", &roll_h_k);
    // pitch_pid_h = pid_init("y_axis", &pitch_h_k);
    // yaw_pid_h = pid_init("z_axis", &yaw_h_k);

    stabilizer_initialized = true;

    return ESP_OK;
}

static float get_pid_output(pid_handle_t pid_handle, float target_value, float current_value) {
    pid_update(pid_handle, target_value, current_value);

    return clampf(pid_handle->output, -1, 1);
}

void positive_axis_stabilizer_reset() {
    pid_reset(roll_pid_v);
    pid_reset(pitch_pid_v);
    pid_reset(yaw_pid_v);

    // pid_reset(roll_pid_h);
    // pid_reset(pitch_pid_h);
    // pid_reset(yaw_pid_h);
}

esp_err_t positive_axis_stabilizer_update(transition_state_t transition_state, axis_duties_t* input_axes, gyro_values_t* gyro_values) {
    if (!stabilizer_initialized) {
        return ESP_FAIL;
    }

    if (transition_state == TRANS_HORIZONTAL) {
        // input_axes->roll = get_pid_output(roll_pid_h, max_roll_rate * input_axes->roll, gyro_values->norm_gyro_x);
        // input_axes->pitch = get_pid_output(pitch_pid_h, max_pitch_rate * input_axes->pitch, gyro_values->norm_gyro_y);
        // input_axes->yaw = get_pid_output(roll_pid_h, max_yaw_rate * input_axes->yaw, gyro_values->norm_gyro_z);
    } else {
        input_axes->roll = get_pid_output(roll_pid_v, max_roll_rads * input_axes->roll, gyro_values->roll_rads);
        input_axes->pitch = get_pid_output(pitch_pid_v, max_pitch_rads * input_axes->pitch, gyro_values->pitch_rads);
        input_axes->yaw = get_pid_output(roll_pid_v, max_yaw_rate * input_axes->yaw, gyro_values->norm_gyro_z);
    }

    return ESP_OK;
}


