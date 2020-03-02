#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "ibus_duplex.h"
#include "servo_control.h"

#define TRANS_DUTY_MIN 1000
#define TRANS_DUTY_MAX 7191


static const char* TAG = "PANDION_BUS_MIXER";

static ibus_duplex_handle_t ibus_handle;
static servo_ctrl_handle_t servo_handle;

static void read_task() {
    ibus_channel_vals_t values;


    while(1) {
        if (ibus_duplex_update(ibus_handle) == ESP_OK) {
            ibus_get_channel_values(ibus_handle, &values);
            ESP_LOGI(TAG, "Vals: %d, %d, %d, %d, %d, %d", values.channels[0], values.channels[1], values.channels[2], values.channels[3], values.channels[4], values.channels[5]);

            int channel_0_duty = ((values.channels[0]-1000) * (TRANS_DUTY_MAX - TRANS_DUTY_MIN) / 1000) + TRANS_DUTY_MIN;

            ESP_LOGI(TAG, "Trans Duty: %d", channel_0_duty);

            // servo_ctrl_set_channel(servo_handle, SERVO_CHANNEL_0, )   
        }


        
    }
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);

    ibus_handle = ibus_duplex_init();
    servo_handle = servo_ctrl_init();

    read_task();
}
