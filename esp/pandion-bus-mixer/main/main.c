#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "ibus_duplex.h"
#include "servo_control.h"

static const char* TAG = "PANDION_BUS_MIXER";

static void read_task(void *arg) {
    ibus_channel_vals_t values;
    ibus_duplex_handle_t handle = (ibus_duplex_handle_t)arg;

    while(1) {
        if (ibus_duplex_update(handle) == ESP_OK) {
            ibus_get_channel_values(handle, &values);
            ESP_LOGI(TAG, "Vals: %d, %d, %d, %d, %d, %d", values.channels[0], values.channels[1], values.channels[2], values.channels[3], values.channels[4], values.channels[5]);


        }
    }
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);

    ibus_duplex_handle_t handle = ibus_duplex_init();

    read_task(handle);
}
