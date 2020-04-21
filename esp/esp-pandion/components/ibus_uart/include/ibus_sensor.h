#pragma once

#include "esp_log.h"
#include "esp_err.h"
#include "ibus_common.h"

typedef enum {
    // TODO: this.
    INTERNAL_POWER = 0x10,

} ibus_sensor_type_t;

typedef uint16_t ibus_sensor_value_t;

typedef struct {
    ibus_sensor_type_t sensor_type;
    ibus_sensor_value_t value;
    uint64_t last_send;
} ibus_sensor_t;

typedef struct ibus_sensor_handle_impl {
    
} *ibus_sensor_handle_t;

ibus_sensor_t ibus_create_sensor(ibus_sensor_type_t type, ibus_sensor_value_t value);

esp_err_t ibus_push_sensor(ibus_sensor_handle_t handle, ibus_sensor_t* sensor);

esp_err_t ibus_sensor_update(ibus_sensor_handle_t handle);
