#include "flight_control.h"

#define MAX_ROLL_THRUST_DIFFERENTIAL 0.15
#define MAX_YAW_THRUST_DIFFERENTIAL 0.15
#define TRANS_DUTY_STEP 0.01

#define MAX_ROLL_RATE 45.0
#define MAX_PITCH_RATE 45.0
#define MAX_YAW_RATE 60.0

// +/- factor from 0 where aft prop input forced to 0.
#define AFT_PROP_DEADZONE 0.01
// Multiplier for aft prop magnitude 0...1.
#define AFT_PROP_SCALAR 0.8

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

static axis_curve_handle_t roll_curve_handle;
static axis_curve_handle_t pitch_curve_handle;
static axis_curve_handle_t yaw_curve_handle;

static axis_pid_constants_t roll_pid_k;
static axis_pid_constants_t pitch_pid_k;
static axis_pid_constants_t yaw_pid_k;

static xQueueHandle timer_queue = NULL;

static ibus_ctrl_handle_t ctrl_handle;
static ibus_sensor_handle_t sensor_handle;
static ibus_sensor_t extv_sensor;

static ibus_ctrl_channel_vals_t* channel_vals;
static transition_state_t transition_state;
static servo_ctrl_handle_t servo_handle;
static dshot_handle_t lw_dshot;
static dshot_handle_t rw_dshot;
static dshot_handle_t aft_dshot;

static float target_trans_duty;
static float current_trans_duty;

#ifdef PANDION_GYRO_ENABLED
static gyro_values_t gyro_values;
static bool stabilization_armed;
#endif

static axis_duties input_axes;

static float clampf(float value, float lower, float upper) {
    return fmin(upper, fmax(lower, value));
}

static float get_channel_duty(uint8_t channel_idx) {
    return (channel_vals->channels[channel_idx] - 1500) / 500.0;
}

static transition_state_t get_transition_state() {
    float trans_channel_val = get_channel_duty(IBUS_RX_CHAN_TRANSITION);
    if (trans_channel_val < -0.3) {
        return TRANS_VERTICAL;
    } else if (trans_channel_val < 0.3) {
        return TRANS_MID;
    }

    return  TRANS_HORIZONTAL;
}

static esp_err_t update_transition_state(bool force) {
    transition_state_t new_trans_state = get_transition_state();

    if (force || new_trans_state != transition_state) {
        ESP_LOGI(TAG, "New transition: %d", new_trans_state);
        transition_state = new_trans_state;
        
        switch(transition_state) {
            case TRANS_UNSET:
                ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
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
    float roll_unit = (input_axes.roll + 1) / 2;
    float throttle_unit = (input_axes.throttle + 1) / 2;
    switch (transition_state)
    {
        case TRANS_UNSET:
            ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
        case TRANS_VERTICAL:
        case TRANS_MID:
            dshot_set_throttle(lw_dshot, clampf(throttle_unit + MAX_ROLL_THRUST_DIFFERENTIAL * input_axes.roll, 0, 1));
            dshot_set_throttle(rw_dshot, clampf(throttle_unit - MAX_ROLL_THRUST_DIFFERENTIAL * input_axes.roll, 0, 1));

            break;
        case TRANS_HORIZONTAL:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RWTILT_CHAN, 1-roll_unit));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, LWTILT_CHAN, 1-roll_unit));
            break;
    }
}

static float reversable_esc_pitch_unit() {
    if (input_axes.pitch <= -AFT_PROP_DEADZONE) {
        // pitch | -1...-DEADZONE => unit | 0.5...0.0
        return AFT_PROP_SCALAR * 0.5 * (-input_axes.pitch - AFT_PROP_DEADZONE) / (1 - AFT_PROP_DEADZONE);
    } else if (input_axes.pitch >= AFT_PROP_DEADZONE) {
        // pitch | +DEADZONE...1 => unit | 0.5...1.0
        return 0.5 + AFT_PROP_SCALAR * 0.5 * (input_axes.pitch - AFT_PROP_DEADZONE) / (1 - AFT_PROP_DEADZONE); 
    } else {
        // pitch in deadzone
        return 0;
    }
}

static void update_pitch() {
    float pitch_unit;

    switch (transition_state)
    {
        case TRANS_UNSET:
        case TRANS_VERTICAL:
        case TRANS_MID:
            pitch_unit = reversable_esc_pitch_unit();
            dshot_set_throttle(aft_dshot, pitch_unit);
            break;
        case TRANS_HORIZONTAL:
            //TODO: if input past threshold, use aft fan?

            pitch_unit = (-input_axes.pitch + 1) / 2;
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, ELEVATOR_CHAN, pitch_unit));
            break;
    }
}

static void update_yaw() {
    float yaw_unit = (input_axes.yaw + 1) / 2;
    float throttle_unit = (input_axes.throttle + 1) / 2;

    switch (transition_state)
    {
        case TRANS_UNSET:
            ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
        case TRANS_VERTICAL:
        case TRANS_MID:
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RWTILT_CHAN, yaw_unit));
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, LWTILT_CHAN, yaw_unit));
            break;
        case TRANS_HORIZONTAL:
            dshot_set_throttle(lw_dshot, clampf(throttle_unit + MAX_YAW_THRUST_DIFFERENTIAL * input_axes.yaw, 0, 1));
            dshot_set_throttle(rw_dshot, clampf(throttle_unit - MAX_YAW_THRUST_DIFFERENTIAL * input_axes.yaw, 0, 1));
            
            ESP_ERROR_CHECK_WITHOUT_ABORT(servo_ctrl_set_channel_duty(servo_handle, RUDDER_CHAN, yaw_unit));
            break;
    }
}

static void update_input_axes() {
    input_axes.roll = get_channel_duty(IBUS_RX_CHAN_ROLL);
    input_axes.pitch = get_channel_duty(IBUS_RX_CHAN_PITCH);
    input_axes.yaw = get_channel_duty(IBUS_RX_CHAN_RUDDER);
    
    input_axes.throttle = get_channel_duty(IBUS_RX_CHAN_THROTTLE);

    if (input_axes.throttle < -0.9) {
        input_axes.throttle = -1.0;
    }

#ifdef PANDION_GYRO_ENABLED
    bool armed = get_channel_duty(IBUS_RX_CHAN_ARM) > 0.5;
    if (!stabilization_armed && armed) {
        pid_reset(roll_pid_handle);
        pid_reset(pitch_pid_handle);
        pid_reset(yaw_pid_handle);
    }
    stabilization_armed = armed;
    if (stabilization_armed) {
        pid_update(roll_pid_handle, MAX_ROLL_RATE * input_axes.roll, gyro_values.norm_gyro_x);
        pid_update(pitch_pid_handle, MAX_PITCH_RATE * input_axes.pitch, gyro_values.norm_gyro_y);
        pid_update(yaw_pid_handle, MAX_YAW_RATE * input_axes.yaw, -gyro_values.norm_gyro_z);

        input_axes.roll = clampf(roll_pid_handle->output, -1, 1);
        input_axes.pitch = clampf(pitch_pid_handle->output, -1, 1);
        input_axes.yaw = clampf(yaw_pid_handle->output, -1, 1);
    }
#endif

    input_axes.roll = axis_curve_calculate(roll_curve_handle, input_axes.roll);
    input_axes.pitch = axis_curve_calculate(pitch_curve_handle, input_axes.pitch);
    input_axes.yaw = axis_curve_calculate(yaw_curve_handle, input_axes.yaw);

    ESP_LOGD(TAG, "Roll %f, Pitch %f, Yaw %f, Throttle %f", input_axes.roll, input_axes.pitch, input_axes.yaw, input_axes.throttle);
}

static void flight_control_update_task(void *arg) {
    battery_meter_update();
    gyro_control_read(&gyro_values);  
    extv_sensor.value = battery_meter_mv() / 10;
    update_input_axes();
    update_transition_state(false);
    update_roll();
    update_pitch();
    update_yaw();

    vTaskDelete(NULL);
}

static void loop_task(void *arg) {
    timer_event_t evt;
    channel_vals = ibus_channel_vals_init();
    uint64_t tick_count = 0;
    while(true) {
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);
        tick_count+=evt.timer_counter_value;

        if (ibus_sensor_update(sensor_handle) == ESP_OK) {
            //
        }

        if(ibus_control_update(ctrl_handle) == ESP_OK) {
            ibus_control_channel_values(ctrl_handle, channel_vals);
            ESP_LOGD(TAG, "Vals: %d, %d, %d, %d, %d, %d", channel_vals->channels[0], channel_vals->channels[1], channel_vals->channels[2], channel_vals->channels[3], channel_vals->channels[4], channel_vals->channels[5]);
            
            xTaskCreate(flight_control_update_task, "flight_control_update_task", 2048, NULL, 5, NULL);
        }
    }

    vTaskDelete(NULL);
}

esp_err_t flight_control_init() {
#ifdef PANDION_GYRO_ENABLED
    esp_err_t ret = gyro_control_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init gyro %02x", ret);
        return ret; 
    }
#endif

    ESP_ERROR_CHECK_WITHOUT_ABORT(battery_meter_init());

    transition_state = TRANS_UNSET;
    current_trans_duty = 1.0;
    target_trans_duty = 1.0;

    ctrl_handle = ibus_control_init(CONFIG_IBUS_CTRL_UART_NUM, CONFIG_IBUS_CTRL_GPIO);
    sensor_handle = ibus_sensor_init(CONFIG_IBUS_SENSOR_UART_NUM, CONFIG_IBUS_SENSOR_RX_GPIO, CONFIG_IBUS_SENSOR_TX_GPIO);
    extv_sensor = ibus_create_sensor(IBUS_TYPE_EXTV, 0);
    ibus_push_sensor(sensor_handle, &extv_sensor);

    servo_ctrl_channel_cfg_t servo_channel_cfgs[SERVO_CHAN_COUNT] = {
        { 1050, 2050, CONFIG_RWTILT_GPIO },
        { 1050, 2050, CONFIG_LWTILT_GPIO },
        { 985, 2015, CONFIG_RWTRANS_GPIO },
        { 985, 2015, CONFIG_LWTRANS_GPIO },
        { 920, 2080, CONFIG_ELEVATOR_GPIO },
        { 1000, 2000, CONFIG_RUDDER_GPIO }
    };

    servo_handle = servo_ctrl_init(servo_channel_cfgs, SERVO_CHAN_COUNT);

    lw_dshot = dshot_init((dshot_cfg){ .gpio_num = CONFIG_LWPROP_GPIO, .rmt_chan = 0, .name = "Left" });
    rw_dshot = dshot_init((dshot_cfg){ .gpio_num = CONFIG_RWPROP_GPIO, .rmt_chan = 1, .name = "Right" });
    aft_dshot = dshot_init((dshot_cfg){ .gpio_num = CONFIG_AFTPROP_GPIO, .rmt_chan = 2, .name = "Aft" });

    roll_pid_k = (axis_pid_constants_t){
        .vertical = {0.002, 0.0005, 0.0025},
        .horizontal = {0.002, 0.0005, 0.00}
    };

    pitch_pid_k = (axis_pid_constants_t){
        .vertical = {0.04, 0.002, 0.0},
        .horizontal = {0.04, 0.002, 0.0}
    };

    yaw_pid_k = (axis_pid_constants_t){
        .vertical = {0.002, 0.0001, 0.0},
        .horizontal = {0.002, 0.0005, 0.0025}
    };

    roll_curve_handle = axis_curve_init(0.5);
    pitch_curve_handle = axis_curve_init(0.2);
    yaw_curve_handle = axis_curve_init(0.8);

    roll_pid_handle = pid_init("x_axis", &roll_pid_k.vertical);
    pitch_pid_handle = pid_init("y_axis", &pitch_pid_k.vertical);
    yaw_pid_handle = pid_init("z_axis", &yaw_pid_k.vertical);


    timer_queue = init_timer();

    xTaskCreate(loop_task, "loop_task", 2048, NULL, 5, NULL);

    return ESP_OK;
}
