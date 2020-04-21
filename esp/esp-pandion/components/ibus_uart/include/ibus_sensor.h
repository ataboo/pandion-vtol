#pragma once

#include "esp_log.h"
#include "esp_err.h"
#include "ibus_common.h"

typedef enum {
    IBUS_TYPE_INTV = 0x00,
    IBUS_TYPE_TEMP = 0x01,
    IBUS_TYPE_RPM = 0x02,
    IBUS_TYPE_EXTV = 0x03,
    IBUS_TYPE_CELL = 0x04,
    IBUS_TYPE_AMPS = 0x05,
    IBUS_TYPE_FUEL = 0x06
} ibus_sensor_type_t;

typedef uint16_t ibus_sensor_value_t;

typedef struct {
    ibus_sensor_type_t sensor_type;
    ibus_sensor_value_t value;
    uint64_t last_send;
} ibus_sensor_t;

typedef struct ibus_sensor_handle_impl {
    
} *ibus_sensor_handle_t;

ibus_sensor_handle_t ibus_create_sensor_handle();

ibus_sensor_t ibus_create_sensor(ibus_sensor_type_t type, ibus_sensor_value_t value);

esp_err_t ibus_push_sensor(ibus_sensor_handle_t handle, ibus_sensor_t* sensor);

esp_err_t ibus_sensor_update(ibus_sensor_handle_t handle);
