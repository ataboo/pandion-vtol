#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "encoder_counter.h"
#include "motor_control.h"
#include "esp_log.h"

#define GPIO_M1A 16
#define GPIO_M1B 17
#define M1_DUTY 50.0

#define GPIO_ENC_1A 22
#define GPIO_ENC_1B 23

static const char* TAG = "ESP_TRANSITION";

void app_main()
{
    motor_control_cfg_t motor_cfg = {
        .gpio_a = GPIO_M1A,
        .gpio_b = GPIO_M1B,
        .gpio_encoder = GPIO_ENC_1A,
        .unit = MCPWM_UNIT_0,
        .timer = MCPWM_TIMER_0,
        .duty_cycle = M1_DUTY
    };

    motor_control_handle_t motor_1 = motor_control_init(&motor_cfg);

    motor_control_move_fwd(motor_1);
    ESP_LOGI(TAG, "Count: %d", motor_control_get_position(motor_1));

    vTaskDelay(2000/portTICK_RATE_MS);
    ESP_LOGI(TAG, "Count: %d", motor_control_get_position(motor_1));

    motor_control_stop(motor_1);
    vTaskDelay(1000/portTICK_RATE_MS);
    ESP_LOGI(TAG, "Count: %d", motor_control_get_position(motor_1));

    motor_control_move_back(motor_1);
    vTaskDelay(2000/portTICK_RATE_MS);
    ESP_LOGI(TAG, "Count: %d", motor_control_get_position(motor_1));

    motor_control_stop(motor_1);
    vTaskDelay(1000/portTICK_RATE_MS);
    ESP_LOGI(TAG, "Count: %d", motor_control_get_position(motor_1));

    vTaskDelay(5000/portTICK_PERIOD_MS);

    esp_restart();
}
