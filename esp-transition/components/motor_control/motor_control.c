#include "motor_control.h"

#define PWM_FREQUENCY 1000
#define DUTY_MODE MCPWM_DUTY_MODE_0
#define COUNTER_MODE MCPWM_UP_COUNTER


struct motor_control_impl {
    mcpwm_config_t *pwm_config;
    motor_control_cfg_t *config;
    encoder_counter_handle_t encoder;
};

motor_control_handle_t motor_control_init(motor_control_cfg_t *config) 
{
    mcpwm_config_t pwm_config = {
        .duty_mode = DUTY_MODE,
        .frequency = PWM_FREQUENCY,
        .counter_mode = COUNTER_MODE,
        .cmpr_a = 0,
        .cmpr_b = 0
    };

    mcpwm_init(config->unit, config->timer, &pwm_config);

    motor_control_handle_t handle = malloc(sizeof(struct motor_control_impl));

    handle->config = config;
    handle->pwm_config = &pwm_config;

    mcpwm_gpio_init(config->unit, MCPWM0A, config->gpio_a);
    mcpwm_gpio_init(config->unit, MCPWM0B, config->gpio_b);

    encoder_cfg_t encoder_cfg = {
        .gpio_num = config->gpio_encoder
    };

    handle->encoder = encoder_counter_init(&encoder_cfg);

    return handle;
}

void motor_control_move_fwd(motor_control_handle_t motor_handle) 
{
    encoder_counter_set_direction(motor_handle->encoder, 1);
    mcpwm_set_duty(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_A, motor_handle->config->duty_cycle);
    mcpwm_set_signal_low(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_B);
    mcpwm_set_duty_type(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_A, DUTY_MODE);
}

void motor_control_move_back(motor_control_handle_t motor_handle) 
{
    encoder_counter_set_direction(motor_handle->encoder, -1);
    mcpwm_set_signal_low(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_A);
    mcpwm_set_duty(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_B, motor_handle->config->duty_cycle);
    mcpwm_set_duty_type(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_B, DUTY_MODE);
}

void motor_control_stop(motor_control_handle_t motor_handle) 
{
    mcpwm_set_signal_low(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_A);
    mcpwm_set_signal_low(motor_handle->config->unit, motor_handle->config->timer, MCPWM_OPR_B);
}

int motor_control_get_position(motor_control_handle_t motor_handle) {
    return encoder_counter_current_count(motor_handle->encoder);
}
