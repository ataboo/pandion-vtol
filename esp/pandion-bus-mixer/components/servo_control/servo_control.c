#include "servo_control.h"

#define SERVO_LEDC_TIMER_NUMBER LEDC_TIMER_0
#define SERVO_LEDC_FREQUENCY 50
#define SERVO_LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define SERVO_LEDC_IDLE_DUTY 4095

typedef enum {
    SERVO_CHANNEL_0 = CONFIG_RWTRANS_GPIO,
    SERVO_CHANNEL_1 = CONFIG_LWTRANS_GPIO,
    SERVO_CHANNEL_COUNT = 2
} servo_gpio_pin;

struct servo_ctrl_handle_impl {
    uint32_t channel_duties[SERVO_CHANNEL_COUNT];
    uint32_t gpio_nums[SERVO_CHANNEL_COUNT];
    ledc_channel_t ledc_channels[SERVO_CHANNEL_COUNT];
};

servo_ctrl_handle_t servo_ctrl_init() {
    servo_ctrl_handle_t handle = (servo_ctrl_handle_t)malloc(sizeof(struct servo_ctrl_handle_impl));

    ledc_timer_config_t ledc_timer = {
        //0...8191
        .duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty
        .freq_hz = SERVO_LEDC_FREQUENCY,                         // frequency of PWM signal
        .speed_mode = SERVO_LEDC_SPEED_MODE,    // timer mode
        .timer_num = SERVO_LEDC_TIMER_NUMBER,             // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };

    ledc_timer_config(&ledc_timer);

    for(int i=0; i<SERVO_CHANNEL_COUNT; i++) {
        ledc_channel_config_t channel_config = {
            .channel    = handle->ledc_channels[i],
            .duty       = SERVO_LEDC_IDLE_DUTY,
            .gpio_num   = handle->gpio_nums[i],
            .speed_mode = SERVO_LEDC_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = SERVO_LEDC_TIMER_NUMBER
        };

        ledc_channel_config(&channel_config);
    }

    return handle;
}

esp_err_t servo_ctrl_set_channel(servo_ctrl_handle_t handle, ledc_channel_t servo_channel, uint32_t duty) {
    handle->channel_duties[servo_channel] = duty;

    return ledc_set_duty(SERVO_LEDC_SPEED_MODE, handle->ledc_channels[servo_channel], duty);
}

esp_err_t servo_ctrl_terminate(servo_ctrl_handle_t handle) {
    for(int i=0; i<SERVO_CHANNEL_COUNT; i++) {
        ledc_stop(SERVO_LEDC_SPEED_MODE, handle->ledc_channels[i], SERVO_LEDC_IDLE_DUTY);
    }

    return ESP_OK;
}
