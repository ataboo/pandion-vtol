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
        ESP_ERROR_CHECK(flight_control_update());
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);

    // flight_control_init();

    dshot_handle_t dshot_handle = dshot_init((dshot_cfg){.rmt_chan = 1, .gpio_num = 23});

    dshot_set_throttle(dshot_handle, 1000);

    vTaskDelay(5000/portTICK_PERIOD_MS);

    // read_task();
}
