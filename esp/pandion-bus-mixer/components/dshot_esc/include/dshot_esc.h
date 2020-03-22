#pragma once

#include "driver/rmt.h"
#include "esp_log.h"
#include "esp_err.h"

typedef struct {
    gpio_num_t gpio_num;
    rmt_channel_id_t rmt_chan;
} dshot_cfg;

typedef struct dshot_handle_impl *dshot_handle_t;

dshot_handle_t dshot_init(dshot_cfg cfg);

esp_err_t dshot_set_throttle(dshot_handle_t handle, uint16_t throttle);

esp_err_t dshot_terminate(dshot_handle_t handle);
