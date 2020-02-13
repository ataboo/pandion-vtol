#include "motor_control.h"

#define PWM_FREQUENCY 50
#define DUTY_MODE MCPWM_DUTY_MODE_1
#define COUNTER_MODE MCPWM_UP_COUNTER


struct motor_control_impl {
    mcpwm_config_t pwm_config;
    struct motor_control_cfg config;

};

// /**
//  * @brief motor moves in backward direction, with duty cycle = duty %
//  */
// static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
// {
//     mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
//     mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
//     mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
// }

// /**
//  * @brief motor stop
//  */
// static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
// {
//     mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
//     mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
// }

motor_control_handle_t motor_control_init(struct motor_control_cfg *config) 
{
    mcpwm_config_t pwm_config {
        .duty_mode = DUTY_MODE,
        .frequency = PWM_FREQUENCY,
        .counter_mode = COUNTER_MODE,
        .cmpr_a = 0,
        .cmpr_b = 0
    };

    mcpwm_init(config.unit, config.timer, &pwm_config);
    // while (1) {
    //     brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 25.0);
    //     vTaskDelay(2000 / portTICK_RATE_MS);

    //     brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    //     vTaskDelay(1000/portTICK_RATE_MS);

    //     brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 25.0);
    //     vTaskDelay(2000 / portTICK_RATE_MS);
        
    //     brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    //     vTaskDelay(2000 / portTICK_RATE_MS);

    motor_control_handle_t handle = malloc(sizeof(struct motor_control_impl));

    handle->config = config;
    handle->pwm_config = pwm_config;

    return handle;
}

void motor_control_move_up(motor_control_handle_t motor_handle) 
{

}

void motor_control_move_fwd(motor_control_handle_t motor_handle) 
{
    mcpwm_set_duty(motor_handle->pwm_config.unit, motor_handle->pwm_config.timer, MCPWM_OPR_A, motor_handle->config.duty_cycle);
    mcpwm_set_signal_low(motor_handle->pwm_config.unit, motor_handle->pwm_config.timer, MCPWM_OPR_B);
}

void motor_control_move_back(motor_control_handle_t motor_handle) 
{
    mcpwm_set_signal_low(motor_handle->pwm_config.unit, motor_handle->pwm_config.timer, MCPWM_OPR_A);
    mcpwm_set_duty(motor_handle->pwm_config.unit, motor_handle->pwm_config.timer, MCPWM_OPR_B, motor_handle->config.duty_cycle);
}

void motor_control_stop(motor_control_handle_t motor_handle) 
{
    mcpwm_set_signal_low(motor_handle->pwm_config.unit, motor_handle->pwm_config.timer, MCPWM_OPR_A);
    mcpwm_set_signal_low(motor_handle->pwm_config.unit, motor_handle->pwm_config.timer, MCPWM_OPR_B);
}
