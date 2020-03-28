#include "flight_control.h"

#define MAX_ROLL_THRUST_DIFFERENTIAL 0.15
#define MAX_YAW_THRUST_DIFFERENTIAL 0.15
#define AFT_PROP_FACTOR 0.2
#define TRANS_DUTY_STEP 0.05

#define MAX_ROLL_RATE 45.0
#define MAX_PITCH_RATE 45.0
#define MAX_YAW_RATE 45.0

typedef struct {
    float roll;
    float pitch;
    float yaw;
    float throttle;
} axis_duties;

typedef struct {
    pid_constants_t vertical;
    pid_constants_t horizontal;
} axis_pid_constants_t;

static const char* TAG = "FLIGHT_CONTROL";

static pid_handle_t roll_pid_handle;
static pid_handle_t pitch_pid_handle;
static pid_handle_t yaw_pid_handle;

static axis_pid_constants_t roll_pid_k;
static axis_pid_constants_t pitch_pid_k;
static axis_pid_constants_t yaw_pid_k;

static ibus_duplex_handle_t ibus_handle;
static ibus_channel_vals_t ibus_values;
static transition_state_t transition_state;
static servo_ctrl_handle_t servo_handle;
static gyro_values_t gyro_values;
static dshot_handle_t lw_dshot;
static dshot_handle_t rw_dshot;
static dshot_handle_t aft_dshot;

static float target_trans_duty;
static float current_trans_duty;

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

static esp_err_t update_transition_state(bool force) {
    transition_state_t new_trans_state = get_transition_state();
    if (!force && new_trans_state == transition_state) {
        return ESP_OK;
    }

    if (force || new_trans_state != transition_state) {
        ESP_LOGI(TAG, "New transition: %d", new_trans_state);
        transition_state = new_trans_state;
        
        switch(transition_state) {
            case TRANS_HORIZONTAL:
                pid_set_gains(roll_pid_handle, &roll_pid_k.horizontal);
                pid_set_gains(pitch_pid_handle, &pitch_pid_k.horizontal);
                pid_set_gains(yaw_pid_handle, &yaw_pid_k.horizontal);
                target_trans_duty = 0.0;
                break;
            case TRANS_MID:
                pid_set_gains(roll_pid_handle, &roll_pid_k.vertical);
                pid_set_gains(pitch_pid_handle, &pitch_pid_k.vertical);
                pid_set_gains(yaw_pid_handle, &yaw_pid_k.vertical);
                target_trans_duty = 0.8;
                break;
            default:
            case TRANS_VERTICAL:
                pid_set_gains(roll_pid_handle, &roll_pid_k.vertical);
                pid_set_gains(pitch_pid_handle, &pitch_pid_k.vertical);
                pid_set_gains(yaw_pid_handle, &yaw_pid_k.vertical);
                target_trans_duty = 1.0;
                break;
        }
    }

    if (!force && current_trans_duty == target_trans_duty) {
        return ESP_OK;
    }

    if (target_trans_duty > current_trans_duty) {
        current_trans_duty += TRANS_DUTY_STEP;
    } else {
        current_trans_duty -= TRANS_DUTY_STEP;
    }

    current_trans_duty = clampf(current_trans_duty, 0, 1);

    return servo_ctrl_set_channel_duty(servo_handle, LWTRANS_CHAN, 1-current_trans_duty) | 
        servo_ctrl_set_channel_duty(servo_handle, RWTRANS_CHAN, current_trans_duty);
}

static void update_roll() {
    float roll_unit = (input_axes.roll - 0.5) * 2;
    switch (transition_state)
    {
        case TRANS_VERTICAL:
        case TRANS_MID:
            dshot_set_throttle(lw_dshot, clampf(input_axes.throttle + MAX_ROLL_THRUST_DIFFERENTIAL * roll_unit, 0, 1));
            dshot_set_throttle(rw_dshot, clampf(input_axes.throttle - MAX_ROLL_THRUST_DIFFERENTIAL * roll_unit, 0, 1));

            break;
        case TRANS_HORIZONTAL:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RWTILT_CHAN, 1-input_axes.roll));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, LWTILT_CHAN, 1-input_axes.roll));
            break;
    }
}

static void update_pitch() {
    switch (transition_state)
    {
        case TRANS_VERTICAL:
        case TRANS_MID:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, AFTPROP_CHAN, input_axes.pitch));
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
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RWTILT_CHAN, input_axes.yaw));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, LWTILT_CHAN, input_axes.yaw));
            break;
        case TRANS_HORIZONTAL:
            dshot_set_throttle(lw_dshot, clampf(input_axes.throttle + MAX_YAW_THRUST_DIFFERENTIAL * yaw_unit, 0, 1));
            dshot_set_throttle(rw_dshot, clampf(input_axes.throttle - MAX_YAW_THRUST_DIFFERENTIAL * yaw_unit, 0, 1));
            
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RUDDER_CHAN, input_axes.yaw));
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

    input_axes.roll = get_channel_duty(IBUS_CHAN_ROLL);
    input_axes.pitch = get_channel_duty(IBUS_CHAN_PITCH);
    input_axes.yaw = get_channel_duty(IBUS_CHAN_RUDDER);
    input_axes.throttle = get_channel_duty(IBUS_CHAN_THROTTLE);

    if (input_axes.throttle < 0.05) {
        input_axes.throttle = 0.0;
    }

    if (stabilization_armed) {
        pid_update(roll_pid_handle, MAX_ROLL_RATE/1000 * (input_axes.roll - 0.5) * 2, gyro_values.norm_gyro_x);
        pid_update(pitch_pid_handle, MAX_PITCH_RATE/1000 * (input_axes.pitch - 0.5) * 2, gyro_values.norm_gyro_y);
        pid_update(yaw_pid_handle, MAX_YAW_RATE/1000 * (input_axes.yaw - 0.5) * 2, -gyro_values.norm_gyro_z);

        input_axes.roll = clampf(roll_pid_handle->output / 2 + 0.5, 0, 1);
        input_axes.pitch = clampf(pitch_pid_handle->output / 2 + 0.5, 0, 1);
        input_axes.yaw = clampf(yaw_pid_handle->output / 2 + 0.5, 0, 1);
    }
}

esp_err_t flight_control_init() {
    esp_err_t ret = gyro_control_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init gyro %02x", ret);
        return ret; 
    }

    transition_state = TRANS_VERTICAL;
    ibus_handle = ibus_duplex_init();

    servo_ctrl_channel_cfg_t servo_channel_cfgs[SERVO_CHAN_COUNT] = {
        { 1000, 2000, CONFIG_RWTILT_GPIO },
        { 1000, 2000, CONFIG_LWTILT_GPIO },
        { 1000, 2000, CONFIG_RWTRANS_GPIO },
        { 1000, 2000, CONFIG_LWTRANS_GPIO },
        { 1000, 2000, CONFIG_ELEVATOR_GPIO },
        { 1000, 2000, CONFIG_RUDDER_GPIO }
    };

    servo_handle = servo_ctrl_init(servo_channel_cfgs, SERVO_CHAN_COUNT);

    lw_dshot = dshot_init((dshot_cfg){ .gpio_num = CONFIG_LWPROP_GPIO, .rmt_chan = 0, .name = "Left" });
    rw_dshot = dshot_init((dshot_cfg){ .gpio_num = CONFIG_RWPROP_GPIO, .rmt_chan = 1, .name = "Right" });
    aft_dshot = dshot_init((dshot_cfg){ .gpio_num = CONFIG_AFTPROP_GPIO, .rmt_chan = 2, .name = "Aft" });

    roll_pid_k = (axis_pid_constants_t){
        .vertical = {0.5, 0.5, 0.1},
        .horizontal = {0.5, 0.5, 0.1}
    };

    pitch_pid_k = (axis_pid_constants_t){
        .vertical = {0.5, 0.5, 0.1},
        .horizontal = {0.5, 0.5, 0.1}
    };

    yaw_pid_k = (axis_pid_constants_t){
        .vertical = {0.5, 0.5, 0.1},
        .horizontal = {0.5, 0.5, 0.1}
    };

    roll_pid_handle = pid_init("x_axis", &roll_pid_k.vertical);
    pitch_pid_handle = pid_init("y_axis", &pitch_pid_k.vertical);
    yaw_pid_handle = pid_init("z_axis", &yaw_pid_k.vertical);

    update_transition_state(true);

    return ESP_OK;
}

esp_err_t flight_control_update() {
    gyro_control_read(&gyro_values);

    esp_err_t ret = ibus_duplex_update(ibus_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = ibus_get_channel_values(ibus_handle, &ibus_values);
    if (ret != ESP_OK) {
        //TODO: fallback values?
        return ret;
    }

    // ESP_LOGI("CONSOLE_PLOTTER", 
    //     "[%f, %f, %f]", 
    //     gyro_values.norm_gyro_x,
    //     gyro_values.norm_gyro_y,
    //     gyro_values.norm_gyro_z
    // );

    // ESP_LOGI("CONSOLE_PLOTTER", 
    //     "[%f, %f, %f]", 
    //     input_axes.yaw,
    //     -100.0,
    //     -100.0
    // );

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
    update_transition_state(false);

    update_roll();
    update_pitch();
    update_yaw();

    return ESP_OK;
}
