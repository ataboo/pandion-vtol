#include "dshot_esc.h"                                  

// https://www.speedgoat.com/help/slrt/page/io_main/refentry_dshot_usage_notes
// https://www.rcgroups.com/forums/showthread.php?2756129-Dshot-testing-a-new-digital-parallel-ESC-throttle-signal
// 0         | disarm
// 1...5     | beep low to high
// 6         | info request
// 7         | rotate forward (10x)
// 8         | rotate backwards (10x)
// 9         | 3d mode off (10x)
// 10        | 3d mode on (10x)
// 11        | ESC setting request
// 12        | save settings (10x)
// 13...47   | reserved
// 48...2047 | 1...2000 throttle

static const char* TAG = "DSHOT_ESC";

static rmt_item32_t rmt_buffer[RMT_BUFFER_LEN];

typedef struct {
    dshot_cfg config;
    rmt_config_t rmt_config;
    uint16_t output_value;
    uint16_t checksum_value;
    uint16_t throttle_value;
} dshot_handle_impl;

static uint8_t dshot_checksum(uint16_t output) {
    uint8_t checksum = 0;
    for(int i=1; i<4; i++) {
        checksum ^= (output >> i*4) & 15;
    }

    return checksum;
}

static void set_rmt_value_buffer(uint16_t output) {
    bool bit_high;
    for(int i=0; i<16; i++) {
        bit_high = (bool)((output >> i) & 1);
        rmt_buffer[15-i].duration0 = bit_high ? DSHOT_1H_TICKS : DSHOT_0H_TICKS;
        rmt_buffer[15-i].duration1 = bit_high ? DSHOT_1L_TICKS : DSHOT_0L_TICKS;
    }
}

dshot_handle_t dshot_init(dshot_cfg cfg) {
    dshot_handle_impl* handle = malloc(sizeof(dshot_handle_impl));

    rmt_config_t rmt_cfg = RMT_DEFAULT_CONFIG_TX(cfg.gpio_num, cfg.rmt_chan);
    rmt_cfg.tx_config.idle_output_en = 1;
    rmt_cfg.clk_div = DSHOT_CLOCK_DIV;
    handle->config = cfg;
    handle->rmt_config = rmt_cfg;

    for(int i=0; i<16; i++) {
        rmt_buffer[i] = (rmt_item32_t) {
            .duration0 = DSHOT_0H_TICKS,
            .level0 = 1,
            .duration1 = DSHOT_0L_TICKS,
            .level1 = 0
        };  
    }

    rmt_buffer[16] = (rmt_item32_t) {
        .duration0 = DSHOT_SPACING_TICKS,
        .level0 = 0,
        .duration1 = 0,
        .level1 = 0
    };

    ESP_ERROR_CHECK(rmt_config(&rmt_cfg));
    ESP_ERROR_CHECK(rmt_driver_install(cfg.rmt_chan, 0, 0));

    ESP_LOGI(TAG, "%s | Installed RMT for D-shot %d", cfg.name, DSHOT_RATE);

    return (dshot_handle_t)handle;
}

esp_err_t dshot_set_output(dshot_handle_t handle, uint16_t output) {
    dshot_handle_impl* handle_impl = (dshot_handle_impl*)handle;

    handle_impl->checksum_value = dshot_checksum(handle_impl->output_value);

    ESP_LOGD(TAG, "%s | Checksum: %d", handle_impl->config.name, handle_impl->checksum_value);

    // Last 4 bits checksum
    handle_impl->output_value |= handle_impl->checksum_value;

    ESP_LOGD(TAG, "%s | Packed output: %d", handle_impl->config.name, handle_impl->output_value);

    set_rmt_value_buffer(handle_impl->output_value);

    ESP_ERROR_CHECK(rmt_write_items(handle_impl->rmt_config.channel, (rmt_item32_t*)rmt_buffer, RMT_BUFFER_LEN, true));
    rmt_set_tx_intr_en(handle_impl->rmt_config.channel, false);
    rmt_set_tx_loop_mode(handle_impl->rmt_config.channel, true);

    return ESP_OK;
}

esp_err_t dshot_set_throttle(dshot_handle_t handle, float throttle) {
    dshot_handle_impl* handle_impl = (dshot_handle_impl*)handle;
    
    if (throttle < 0 || throttle > 1) {
        ESP_LOGE(TAG, "%s | Throttle out of range: %f", handle_impl->config.name, throttle);
        return ESP_ERR_INVALID_SIZE;
    }

    ESP_LOGD(TAG, "%s | Throttle factor: %f", handle_impl->config.name, throttle);

    handle_impl->throttle_value = (uint16_t)(throttle * 1999 + 48);

    ESP_LOGD(TAG, "%s | Throttle value: %d", handle_impl->config.name, handle_impl->throttle_value);

    // Shift to start of 16 bit output.  0...10 throttle, 11 telemetry request, 12...15 checksum
    handle_impl->output_value = handle_impl->throttle_value << 5;

    return dshot_set_output(handle, handle_impl->output_value);
}

esp_err_t dshot_terminate(dshot_handle_t handle) {
    dshot_handle_impl* handle_impl = (dshot_handle_impl*)handle;

    rmt_driver_uninstall(handle_impl->rmt_config.channel);
    free(handle);

    return ESP_OK;
}
