#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "ibus_duplex.h"
#include "servo_control.h"
#include "gyro_control.h"
#include "esp_log.h"

typedef enum {
    RWTILT_CHAN = 0,
    LWTILT_CHAN,
    RWTRANS_CHAN,
    LWTRANS_CHAN,
    RWPROP_CHAN,
    LWPROP_CHAN,
    AFTPROP_CHAN,
    ELEVATOR_CHAN,
    // RUDDER_CHAN,
    SERVO_CHAN_COUNT
} servo_ctrl_channel_t;


static const char* TAG = "PANDION_BUS_MIXER";

static ibus_duplex_handle_t ibus_handle;
static servo_ctrl_handle_t servo_handle;

static void read_task() {
    ibus_channel_vals_t values;

    while(1) {
        if (ibus_duplex_update(ibus_handle) == ESP_OK) {
            ibus_get_channel_values(ibus_handle, &values);

            // 0 = Roll
            // 1 = Pitch
            // 2 = Throttle
            // 3 = Rudder
            // 4 = Transition
            // 5 = Other Switch

            ESP_LOGI(TAG, "Vals: 0|Roll %d, 1|Pitch %d, 2|Throttle %d, 3|Rudder %d, 4|Transition %d, 5|Other %d", values.channels[0], values.channels[1], values.channels[2], values.channels[3], values.channels[4], values.channels[5]);

            servo_ctrl_set_channel_duty(servo_handle, RWTILT_CHAN, (float)((values.channels[3] - 1000) / 1000.0));
            servo_ctrl_set_channel_duty(servo_handle, LWTILT_CHAN, (float)((values.channels[3] - 1000) / 1000.0));
            servo_ctrl_set_channel_duty(servo_handle, RWTRANS_CHAN, (float)((values.channels[4] - 1000) / 1000.0));
            servo_ctrl_set_channel_duty(servo_handle, LWTRANS_CHAN, (float)((values.channels[4] - 1000) / 1000.0));
            servo_ctrl_set_channel_duty(servo_handle, RWPROP_CHAN, (float)((values.channels[2] - 1000) / 1000.0));
            servo_ctrl_set_channel_duty(servo_handle, LWPROP_CHAN, (float)((values.channels[2] - 1000) / 1000.0));
        }
    }
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);

    // servo_ctrl_channel_cfg_t servo_channel_cfgs[SERVO_CHAN_COUNT] = {
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_RWTILT_GPIO
    //     },
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_LWTILT_GPIO
    //     },
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_RWTRANS_GPIO
    //     },
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_LWTRANS_GPIO
    //     },
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_RWPROP_GPIO
    //     },
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_LWPROP_GPIO
    //     },
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_AFTPROP_GPIO
    //     },
    //     {
    //         .min_us = 1000,
    //         .max_us = 2000,
    //         .gpio_num = CONFIG_ELEVATOR_GPIO
    //     },
    //     // {
    //     //     .min_us = 1000,
    //     //     .max_us = 2000,
    //     //     .gpio_num = CONFIG_RUDDER_GPIO
    //     // }
    // }; 

    // ibus_handle = ibus_duplex_init();
    // servo_handle = servo_ctrl_init(servo_channel_cfgs, SERVO_CHAN_COUNT);

    if (gyro_control_init() != ESP_OK || gyro_check_status() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to MPU6050 Gyro");
        abort();
    }

    ESP_LOGI(TAG, "Connected to MPU6050 Gyro successfully.");

    gyro_values_t gyro_vals = {};

    while(1) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(gyro_control_read(&gyro_vals));
        ESP_LOGI("CONSOLE_PLOTTER", "[%8.5f,%8.5f,%8.5f]", gyro_vals.roll_rads, gyro_vals.pitch_rads, gyro_vals.yaw_rads);
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // read_task();
}
