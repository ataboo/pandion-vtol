#include "flight_control.h"

#define MAX_ROLL_THRUST_DIFFERENTIAL 0.5
#define MAX_YAW_THRUST_DIFFERENTIAL 0.5
#define AFT_PROP_FACTOR 0.5

#define CONCAT_PID_CONFIG(TRANS, AXIS)  CONFIG_PID_ ## TRANS ## _ ## AXIS ## _GAIN

 #define SET_PID_GAINS(TRANS) { \
     pid_set_gains(pitch_pid_handle, CONCAT_PID_CONFIG(TRANS, PX), CONCAT_PID_CONFIG(TRANS, PX), CONCAT_PID_CONFIG(TRANS, PX)); \
     pid_set_gains(pitch_pid_handle, CONCAT_PID_CONFIG(TRANS, PY), CONCAT_PID_CONFIG(TRANS, PY), CONCAT_PID_CONFIG(TRANS, PY)); \
     pid_set_gains(pitch_pid_handle, CONCAT_PID_CONFIG(TRANS, PZ), CONCAT_PID_CONFIG(TRANS, PZ), CONCAT_PID_CONFIG(TRANS, PZ)); \
}

typedef struct {
    float roll;
    float pitch;
    float yaw;
    float throttle;
} axis_duties;

static const char* TAG = "FLIGHT_CONTROL";

static pid_handle_t roll_pid_handle;
static pid_handle_t pitch_pid_handle;
static pid_handle_t yaw_pid_handle;

static ibus_duplex_handle_t ibus_handle;
static ibus_channel_vals_t ibus_values;
static transition_state_t transition_state;
static servo_ctrl_handle_t servo_handle;
static gyro_values_t gyro_values;

static bool stabilization_armed;

static axis_duties input_axes;

static float clampf(float value, float lower, float upper) {
    return fmin(upper, fmax(lower, value));
}

static float get_channel_duty(uint8_t channel_idx) {
    return (ibus_values.channels[channel_idx] - 1000) / 1000.0;
}

static transition_state_t get_transition_state() {
    float trans_channel_val = get_channel_duty(IBUS_CHAN_TRANSITION);
    if (trans_channel_val < 0.3) {
        return TRANS_VERTICAL;
    } else if (trans_channel_val < 0.6) {
        return TRANS_MID;
    }

    return  TRANS_HORIZONTAL;
}

static esp_err_t update_transition_state() {
    transition_state_t new_trans_state = get_transition_state();
    if (new_trans_state == transition_state) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "New transition: %d", new_trans_state);
    transition_state = new_trans_state;
    
    float trans_duty;
    switch(transition_state) {
        case TRANS_HORIZONTAL:
            SET_PID_GAINS(HORIZ);
            trans_duty = 0.0;
            break;
        case TRANS_MID:
            SET_PID_GAINS(VERT);
            trans_duty = 0.8;
            break;
        default:
        case TRANS_VERTICAL:
            SET_PID_GAINS(VERT);
            trans_duty = 1.0;
            break;
    }

    return servo_ctrl_set_channel_duty(servo_handle, LWTRANS_CHAN, 1-trans_duty) | 
        servo_ctrl_set_channel_duty(servo_handle, RWTRANS_CHAN, trans_duty);
}

static void update_roll() {
    float roll_unit = (input_axes.roll - 0.5) * 2;
    switch (transition_state)
    {
        case TRANS_VERTICAL:
        case TRANS_MID:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(
                servo_handle, 
                RWPROP_CHAN, 
                clampf(input_axes.throttle - MAX_ROLL_THRUST_DIFFERENTIAL * roll_unit, 0, 1)
            ));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(
                servo_handle, 
                LWPROP_CHAN, 
                clampf(input_axes.throttle + MAX_ROLL_THRUST_DIFFERENTIAL * roll_unit, 0, 1)
            ));
            break;
        case TRANS_HORIZONTAL:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RWTILT_CHAN, 1 - input_axes.roll));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, LWTILT_CHAN, input_axes.roll));
            break;
    }
}

static void update_pitch() {
    switch (transition_state)
    {
        case TRANS_VERTICAL:
        case TRANS_MID:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(
                servo_handle, 
                AFTPROP_CHAN, 
                input_axes.pitch
            ));
            break;
        case TRANS_HORIZONTAL:
            //TODO: if input past threshold, use aft fan?
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, ELEVATOR_CHAN, input_axes.pitch));
            break;
    }
}

static void update_yaw() {
    float yaw_unit = (input_axes.yaw - 0.5) * 2;

    switch (transition_state)
    {
        case TRANS_VERTICAL:
        case TRANS_MID:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RWTILT_CHAN, 1 - input_axes.yaw));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, LWTILT_CHAN, input_axes.yaw));
            break;
        case TRANS_HORIZONTAL:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RWPROP_CHAN, clampf(input_axes.throttle - MAX_YAW_THRUST_DIFFERENTIAL * yaw_unit, 0, 1)));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, LWPROP_CHAN, clampf(input_axes.throttle + MAX_YAW_THRUST_DIFFERENTIAL * yaw_unit, 0, 1)));
            
            // TODO: rudder
            break;
    }
}

static void update_input_axes() {
    bool armed = get_channel_duty(IBUS_CHAN_ARM) > 0.5;
    if (!stabilization_armed && armed) {
        pid_reset(roll_pid_handle);
        pid_reset(pitch_pid_handle);
        pid_reset(yaw_pid_handle);
    }
    stabilization_armed = armed;

    if (stabilization_armed) {
        pid_update(roll_pid_handle, CONFIG_MAX_ROLL_RATE/1000 * get_channel_duty(IBUS_CHAN_ROLL), gyro_values.norm_gyro_y);
        pid_update(pitch_pid_handle, CONFIG_MAX_PITCH_RATE/1000 * get_channel_duty(IBUS_CHAN_PITCH), gyro_values.norm_gyro_x);
        pid_update(yaw_pid_handle, CONFIG_MAX_YAW_RATE/1000 * get_channel_duty(IBUS_CHAN_RUDDER), gyro_values.norm_gyro_z);

        input_axes.roll = clampf(roll_pid_handle->output / CONFIG_MAX_ROLL_RATE, -1, 1);
        input_axes.pitch = clampf(pitch_pid_handle->output / CONFIG_MAX_PITCH_RATE, -1, 1);
        input_axes.yaw = clampf(yaw_pid_handle->output / CONFIG_MAX_YAW_RATE, -1, 1);
        input_axes.throttle = get_channel_duty(IBUS_CHAN_THROTTLE);
    } else {
        input_axes.roll = get_channel_duty(IBUS_CHAN_ROLL);
        input_axes.pitch = get_channel_duty(IBUS_CHAN_PITCH);
        input_axes.yaw = get_channel_duty(IBUS_CHAN_RUDDER);
        input_axes.throttle = get_channel_duty(IBUS_CHAN_THROTTLE);
    }
}

esp_err_t flight_control_init() {
    transition_state = TRANS_VERTICAL;
    ibus_handle = ibus_duplex_init();
    ESP_ERROR_CHECK(gyro_control_init());

    servo_ctrl_channel_cfg_t servo_channel_cfgs[SERVO_CHAN_COUNT] = {
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_RWTILT_GPIO
        },
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_LWTILT_GPIO
        },
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_RWTRANS_GPIO
        },
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_LWTRANS_GPIO
        },
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_RWPROP_GPIO
        },
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_LWPROP_GPIO
        },
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_AFTPROP_GPIO
        },
        {
            .min_us = 1000,
            .max_us = 2000,
            .gpio_num = CONFIG_ELEVATOR_GPIO
        },
        // {
        //     .min_us = 1000,
        //     .max_us = 2000,
        //     .gpio_num = CONFIG_RUDDER_GPIO
        // }
    }; 

    servo_handle = servo_ctrl_init(servo_channel_cfgs, SERVO_CHAN_COUNT);

    roll_pid_handle = pid_init("x_axis", CONFIG_PID_VERT_PX_GAIN, CONFIG_PID_VERT_IX_GAIN, CONFIG_PID_VERT_DX_GAIN);
    pitch_pid_handle = pid_init("y_axis", CONFIG_PID_VERT_PY_GAIN, CONFIG_PID_VERT_IY_GAIN, CONFIG_PID_VERT_DY_GAIN);
    yaw_pid_handle = pid_init("z_axis", CONFIG_PID_VERT_PZ_GAIN, CONFIG_PID_VERT_IZ_GAIN, CONFIG_PID_VERT_DZ_GAIN);

    return ESP_OK;
}

esp_err_t flight_control_update() {
    gyro_control_read(&gyro_values);

    //TODO this crashes on fail!
    esp_err_t ret = ibus_duplex_update(ibus_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = ibus_get_channel_values(ibus_handle, &ibus_values);
    if (ret != ESP_OK) {
        return ret;
    }

    // ESP_LOGI("CONSOLE_PLOT", 
    //     "[%d, %d, %d, %d, %d, %d]", 
    //     ibus_values.channels[0],
    //     ibus_values.channels[1],
    //     ibus_values.channels[2],
    //     ibus_values.channels[3],
    //     ibus_values.channels[4],    
    //     ibus_values.channels[5],
    // );

    update_input_axes();
    update_transition_state();

    update_roll();
    update_pitch();
    update_yaw();

    return ESP_OK;
}
