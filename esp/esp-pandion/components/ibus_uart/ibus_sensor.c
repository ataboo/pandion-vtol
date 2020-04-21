#include "ibus_sensor.h"

typedef struct {
    int sensor_count;
    uint8_t read_buffer[IBUS_UART_BUFFER_SIZE];
    ibus_sensor_t* sensors;
} ibus_sensor_handle_impl;

ibus_sensor_t ibus_create_sensor(ibus_sensor_type_t type, ibus_sensor_value_t value) {
    ibus_sensor_t sensor = {
        .sensor_type = type,
        .value = value
    };

    return sensor;
}

esp_err_t ibus_push_sensor(ibus_sensor_handle_t handle, ibus_sensor_t* sensor) {
    ibus_sensor_handle_impl* handle_impl = (ibus_sensor_handle_impl*) handle;
    
    if (handle_impl->sensor_count >= IBUS_MAX_SENSOR_COUNT) {
        return ESP_FAIL;
    }

    handle_impl->sensors[handle_impl->sensor_count++] = *sensor;

    return ESP_OK;
}

esp_err_t parse_sensor_input(ibus_sensor_handle_impl* handle_impl, size_t len) {
    if (len != 4) {
        ESP_LOGW(TAG, "Length of %d invalid for IBus sensor.", len);
        return ESP_FAIL;
    }
    
    esp_err_t ret = ibus_test_checksum(handle_impl->read_buffer);
    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t addr = ret[0] && 0x0f;
    uint8_t command = ret[0] && 0xf0;

    if (addr > handle_impl->sensor_count)

    switch(command) {
        case IBUS_CMD_SENSOR_DISCOVER:
            ESP_LOGD(TAG, "Received sensor discover for %d", addr);
        case IBUS_CMD_SENSOR_TYPE:
            ESP_LOGD(TAG, "Received sensor type for %d", addr);
        case IBUS_CMD_SENSOR_SEND:
            ESP_LOGD(TAG, "Received sensor send for %d", addr);
        default:
            ESP_LOG_W(TAG, "Unsupported IBus sensor command: %x", ret[1]);
    }

    return ESP_OK;
}

esp_err_t ibus_sensor_update(ibus_sensor_handle_t handle) {
    ibus_sensor_handle_impl* handle_impl = (ibus_sensor_handle_impl*) handle;
    if (handle_impl->sensor_count == 0 ) {
        return ESP_OK;
    }

    size_t len;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(CONFIG_IBUS_SENSOR_UART_NUM, (size_t*)&len));
    if (len > 0) {
        len = uart_read_bytes(CONFIG_IBUS_SENSOR_UART_NUM, handle_impl->read_buffer, IBUS_UART_BUFFER_SIZE, 5 / portTICK_RATE_MS);

        return parse_sensor_input(handle);
    }

    return ESP_FAIL;
}

