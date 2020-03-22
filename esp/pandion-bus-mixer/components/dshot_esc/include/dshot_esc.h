#pragma once

#include "driver/rmt.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_err.h"

#if CONFIG_DSHOT_RATE600
    #define DSHOT_CLOCK_DIV  1
    #define DSHOT_RATE 600
#elif CONFIG_DSHOT_RATE300
    #define RMT_CLOCK_DIV  2
    #define DSHOT_RATE 300
#else
    #define RMT_CLOCK_DIV 4
    #define DSHOT_RATE 150
#endif

#define RMT_BUFFER_LEN 17

#define DSHOT_MAX_NAME_LEN 32

// 133 / 80Mhz = 1663ns
#define DSHOT_BIT_TICKS 133

// 100 / 80Mhz = 1250ns 
#define DSHOT_1H_TICKS 100
// 33 / 80Mhz =  413ns
#define DSHOT_1L_TICKS DSHOT_BIT_TICKS-DSHOT_1H_TICKS

// 50 / 80Mhz = 625ns
#define DSHOT_0H_TICKS 50
// 83 / 80Mhz = 1038ns
#define DSHOT_0L_TICKS DSHOT_BIT_TICKS-DSHOT_0H_TICKS

#define DSHOT_SPACING_TICKS DSHOT_BIT_TICKS * 32     

typedef struct {
    gpio_num_t gpio_num;
    rmt_channel_id_t rmt_chan;
    char name[DSHOT_MAX_NAME_LEN];
} dshot_cfg;

typedef struct dshot_handle_impl *dshot_handle_t;

dshot_handle_t dshot_init(dshot_cfg cfg);

esp_err_t dshot_set_throttle(dshot_handle_t handle, float throttle);

esp_err_t dshot_set_output(dshot_handle_t handle, uint16_t output);

esp_err_t dshot_terminate(dshot_handle_t handle);
