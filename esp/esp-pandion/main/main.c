#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "flight_control.h"
#include "dshot_esc.h"
#include "ibus_common.h"

static const char* TAG = "PANDION_BUS_MIXER";

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);
    // esp_log_level_set("*", ESP_LOG_DEBUG);

    flight_control_init();
}
