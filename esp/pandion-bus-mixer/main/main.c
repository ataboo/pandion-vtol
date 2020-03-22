#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "flight_control.h"
#include "esp_log.h"
#include "dshot_esc.h"

static const char* TAG = "PANDION_BUS_MIXER";

static void read_task() {
    while(1) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(flight_control_update());
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);
    // esp_log_level_set("*", ESP_LOG_DEBUG);


    if(flight_control_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init flight control.");
        vTaskDelay(10000/portTICK_PERIOD_MS);
        abort();
    }

    // dshot_handle_t dshot_handle = dshot_init((dshot_cfg){.rmt_chan = 1, .gpio_num = 23, .name = "Left"});

    // dshot_set_throttle(dshot_handle, 0.5);

    // vTaskDelay(5000/portTICK_PERIOD_MS);

    read_task();
}
