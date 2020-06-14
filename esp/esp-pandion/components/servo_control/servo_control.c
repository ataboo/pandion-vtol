#include "servo_control.h"

#define SERVO_LEDC_TIMER_NUMBER LEDC_TIMER_0
#define SERVO_LEDC_FREQUENCY 50
#define SERVO_LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE

// 0...65535
#define SERVO_DUTY_RESOLUTION LEDC_TIMER_16_BIT
#define SERVO_DUTY_MAX 65535

// static const char* TAG = "PANDION_SERVO_CONTROL";

typedef struct {
    ledc_channel_t ledc_channel;
    uint32_t min_duty;
    uint32_t max_duty;
    uint32_t current_duty;
} servo_ctrl_channel_state_t;

struct servo_ctrl_handle_impl {
    int channel_count;
    servo_ctrl_channel_state_t* channel_states;
};

servo_ctrl_handle_t servo_ctrl_init(servo_ctrl_channel_cfg_t* channel_cfgs, int channel_count) {
    servo_ctrl_handle_t handle = (servo_ctrl_handle_t)malloc(sizeof(struct servo_ctrl_handle_impl));
    handle->channel_states = (servo_ctrl_channel_state_t*)malloc(sizeof(servo_ctrl_channel_state_t) * channel_count);
    handle->channel_count = channel_count;

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = SERVO_DUTY_RESOLUTION,
        .freq_hz = SERVO_LEDC_FREQUENCY,
        .speed_mode = SERVO_LEDC_SPEED_MODE,   
        .timer_num = SERVO_LEDC_TIMER_NUMBER,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    for(int i=0; i<channel_count; i++) {
        servo_ctrl_channel_state_t state = {
            .ledc_channel = i,
            .min_duty = (uint32_t)(channel_cfgs[i].min_us * SERVO_DUTY_MAX / 20000),
            .max_duty = (uint32_t)(channel_cfgs[i].max_us * SERVO_DUTY_MAX / 20000),
        };
        state.current_duty = (state.max_duty - state.min_duty) / 2 + state.min_duty;

        handle->channel_states[i] = state;

        ledc_channel_config_t channel_config = {
            .channel    = i,
            .duty       = state.current_duty,
            .gpio_num   = channel_cfgs[i].gpio_num,
            .speed_mode = SERVO_LEDC_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = SERVO_LEDC_TIMER_NUMBER
        };

        ledc_channel_config(&channel_config);
    }

    return handle;
}

esp_err_t servo_ctrl_set_channel_duty(servo_ctrl_handle_t handle, ledc_channel_t servo_channel, float fl_duty) {
    servo_ctrl_channel_state_t state = handle->channel_states[servo_channel];  

    uint32_t duty = (uint32_t) (fl_duty * (state.max_duty - state.min_duty) + state.min_duty); 
    state.current_duty = duty;

    // ESP_LOGI(TAG, "Duties min: %d, max: %d, current: %d", state.min_duty, state.max_duty, state.current_duty);

    return ledc_set_duty(SERVO_LEDC_SPEED_MODE, servo_channel, duty) || ledc_update_duty(SERVO_LEDC_SPEED_MODE, servo_channel);
}

esp_err_t servo_ctrl_terminate(servo_ctrl_handle_t handle) {
    for(int i=0; i<handle->channel_count; i++) {
        ledc_stop(SERVO_LEDC_SPEED_MODE, i, SERVO_DUTY_MAX * 2/ 40);
    }

    return ESP_OK;
}
