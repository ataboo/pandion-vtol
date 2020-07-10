#include "positive_axis_stabilizer.h"

static pid_handle_t roll_pid_v;
static pid_handle_t pitch_pid_v;
static pid_handle_t yaw_pid_v;

static float max_roll_rads = PI / 4;
static float max_pitch_rads = PI / 4;
static float max_yaw_rate = 45.0;

static const char* TAG = "PANDION_POS_STABILITY";

static bool stabilizer_initialized = false;

esp_err_t positive_axis_stabilizer_init() {
    if (stabilizer_initialized) {
        return ESP_FAIL;
    }

    pid_constants_t roll_v_k = { config_db_get_float_def("pg_roll_vp", 0.002), config_db_get_float_def("pg_roll_vi", 0.0005), config_db_get_float_def("pg_roll_vd", 0.0025) };
    pid_constants_t pitch_v_k = { config_db_get_float_def("pg_pitch_vp", 0.04), config_db_get_float_def("pg_pitch_vi", 0.002), config_db_get_float_def("pg_pitch_vd", 0.0) };
    pid_constants_t yaw_v_k = { config_db_get_float_def("pg_yaw_vp", 0.002), config_db_get_float_def("pg_yaw_vi", 0.0001), config_db_get_float_def("pg_yaw_vd", 0.0) };

    roll_pid_v = pid_init("x_axis", &roll_v_k);
    pitch_pid_v = pid_init("y_axis", &pitch_v_k);
    yaw_pid_v = pid_init("z_axis", &yaw_v_k);

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
}

esp_err_t positive_axis_stabilizer_update(transition_state_t transition_state, axis_duties_t* input_axes, gyro_values_t* gyro_values) {
    if (!stabilizer_initialized) {
        return ESP_FAIL;
    }

    if (transition_state == TRANS_HORIZONTAL) {
        ESP_LOGE(TAG, "Positive stability not supported in horizontal state.");
        return ESP_FAIL;
    } else {
        input_axes->roll = get_pid_output(roll_pid_v, max_roll_rads * input_axes->roll, gyro_values->roll_rads);
        input_axes->pitch = get_pid_output(pitch_pid_v, max_pitch_rads * input_axes->pitch, gyro_values->pitch_rads);
        input_axes->yaw = get_pid_output(roll_pid_v, max_yaw_rate * input_axes->yaw, gyro_values->norm_gyro_z);
    }

    return ESP_OK;
}


