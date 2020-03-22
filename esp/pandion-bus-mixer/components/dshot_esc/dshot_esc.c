#include "dshot_esc.h"

#define RMT_DIVIDER    2
#define RMT_DURATION  12.5
#define RMT_BUFFER_LEN 2

// #define PULSE_T0H (  625 / (DURATION * DIVIDER));
// #define PULSE_T1H (  1250 / (DURATION * DIVIDER));
// #define PULSE_T0L (  1045 / (DURATION * DIVIDER));
// #define PULSE_T1L (  420 / (DURATION * DIVIDER));

#define PULSE_T0H_RAW 625
#define PULSE_T1H_RAW 1250
#define PULSE_T0L_RAW 1045
#define PULSE_T1L_RAW 420

static const char* TAG = "DSHOT_ESC";

static rmt_item32_t rmt_buffer[RMT_BUFFER_LEN];

typedef struct {
    dshot_cfg config;
    rmt_config_t rmt_config;
} dshot_handle_impl;

static uint8_t dshot_checksum(uint16_t output) {
    uint8_t checksum = 0;
    for(int i=0; i<=8; i+=4) {
        checksum ^= (output >> i) & 15;
    }

    return checksum;
}

dshot_handle_t dshot_init(dshot_cfg cfg) {
    dshot_handle_impl* handle = malloc(sizeof(dshot_handle_impl));

    rmt_config_t rmt_cfg = RMT_DEFAULT_CONFIG_TX(cfg.gpio_num, cfg.rmt_chan);
    rmt_cfg.tx_config.loop_en = 1;
    rmt_cfg.clk_div = RMT_DIVIDER;

    handle->config = cfg;
    handle->rmt_config = rmt_cfg;

    ESP_ERROR_CHECK(rmt_config(&rmt_cfg));
    ESP_ERROR_CHECK(rmt_driver_install(cfg.rmt_chan, 0, 0));

    return (dshot_handle_t)handle;
}

esp_err_t dshot_set_throttle(dshot_handle_t handle, uint16_t throttle) {
    dshot_handle_impl* handle_impl = (dshot_handle_impl*)handle;

    if (throttle > 1999) {
        ESP_LOGE(TAG, "throttle out of range: %d", throttle);
        return ESP_ERR_INVALID_SIZE;
    }

    // https://www.speedgoat.com/help/slrt/page/io_main/refentry_dshot_usage_notes
    // 0         | disarm
    // 1...47    | reserved
    // 48...2047 | 0...1999 throttle 
    throttle += 48;

    // First 11 bits throttle 
    uint16_t output = throttle << 5;

    // 12'th bit 0 for no telemetry request

    // Last 4 bits checksum
    output |= dshot_checksum(output);
    
    rmt_buffer[0] = (rmt_item32_t){
        .duration0 = 1000,
        .level0 = 1,
        .duration1 = 1000,
        .level1 = 0
    };

    rmt_buffer[1] = (rmt_item32_t){
        .duration0 = 1000,
        .level0 = 0,
        .duration1 = 1000,
        .level1 = 0
    };

    ESP_ERROR_CHECK(rmt_write_items(handle_impl->rmt_config.channel, (rmt_item32_t*)rmt_buffer, RMT_BUFFER_LEN, true));
    
    return ESP_OK;
}

esp_err_t dshot_terminate(dshot_handle_t handle) {
    dshot_handle_impl* handle_impl = (dshot_handle_impl*)handle;

    rmt_driver_uninstall(handle_impl->rmt_config.channel);
    free(handle);

    return ESP_OK;
}
