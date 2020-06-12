#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "flight_control.h"
#include "dshot_esc.h"
#include "ibus_common.h"
#include "wifi_ap.h"
#include "tcp_server.h"

static const char* TAG = "PANDION_BUS_MIXER";

static void handle_command(tcp_command_packet_t in, tcp_command_packet_t* out) {
    ESP_LOGI("MAIN", "Got payload: %d, %d", in.payload[0], in.payload[1]);
    out->command = 0x0102;
    out->payload[0] = 0x03;
    out->payload[1] = 0x04;
    out->length = 4;
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);
    // esp_log_level_set("*", ESP_LOG_DEBUG);

    // flight_control_init();

    wifi_ap_init();
    tcp_server_init();
}
