#include "ibus_sensor.h"

static const char* TAG = "IBUS_SENSOR";

typedef struct {
    int sensor_count;
    uint8_t uart_buffer[IBUS_UART_BUFFER_SIZE];
    ibus_sensor_t sensors[IBUS_MAX_SENSOR_COUNT];
} ibus_sensor_handle_impl;

ibus_sensor_handle_t ibus_create_sensor_handle() {
    ibus_sensor_handle_impl* handle_impl = malloc(sizeof(ibus_sensor_handle_impl));
    handle_impl->sensor_count = 0;

    return (ibus_sensor_handle_t)handle_impl;
}

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

esp_err_t parse_sensor_input(ibus_sensor_handle_impl* handle_impl) {
    uint8_t len = handle_impl->uart_buffer[0];
    if (len != 4) {
        ESP_LOGW(TAG, "Length of %d invalid for IBus sensor.", len);
        return ESP_FAIL;
    }
    
    esp_err_t ret = ibus_test_checksum(handle_impl->uart_buffer);
    if (ret != ESP_OK) {
        return ret;
    }

    // ESP_LOGI(TAG, "%.2x, %.2x, %.2x, %.2x", handle_impl->uart_buffer[0], handle_impl->uart_buffer[1], handle_impl->uart_buffer[2], handle_impl->uart_buffer[3]);

    uint8_t addr = handle_impl->uart_buffer[1] & 0x0F;
    uint8_t command = handle_impl->uart_buffer[1] & 0xF0;

    if (addr > handle_impl->sensor_count) {
        return ESP_FAIL;
    }

    uint16_t check_sum = ibus_calculate_checksum(handle_impl->uart_buffer);

    switch(command) {
        case IBUS_CMD_SENSOR_DISCOVER:
            ESP_LOGI(TAG, "Received sensor discover for %d", addr);
            handle_impl->uart_buffer[0] = 0x04;
            handle_impl->uart_buffer[1] = command + addr;
            handle_impl->uart_buffer[2] = check_sum & 0xFF;
            handle_impl->uart_buffer[3] = check_sum >> 8;
            break;
        case IBUS_CMD_SENSOR_TYPE:
            ESP_LOGI(TAG, "Received sensor type for %d", addr);
            handle_impl->uart_buffer[0] = 0x06;
            handle_impl->uart_buffer[1] = command + addr;
            handle_impl->uart_buffer[2] = handle_impl->sensors[addr].sensor_type;
            handle_impl->uart_buffer[3] = 0x02;
            handle_impl->uart_buffer[4] = check_sum & 0xFF;
            handle_impl->uart_buffer[5] = check_sum >> 8;
            break;
        case IBUS_CMD_SENSOR_SEND:
            // ESP_LOGI(TAG, "Received sensor send for %d", addr);
            handle_impl->uart_buffer[0] = 0x06;
            handle_impl->uart_buffer[1] = command + addr;
            handle_impl->uart_buffer[2] = handle_impl->sensors[addr].value & 0xFF;
            handle_impl->uart_buffer[3] = handle_impl->sensors[addr].value << 8;
            handle_impl->uart_buffer[4] = check_sum & 0xFF;
            handle_impl->uart_buffer[5] = check_sum >> 8;
            break;
        default:
            ESP_LOGW(TAG, "Unsupported IBus sensor command: %x", handle_impl->uart_buffer[1]);
            return ESP_FAIL;
    }


    int n = uart_write_bytes(CONFIG_IBUS_SENSOR_UART_NUM, (char*)handle_impl->uart_buffer, handle_impl->uart_buffer[0]);
    if (n == handle_impl->uart_buffer[0]) {
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to set UART");
    return ESP_FAIL;
}

esp_err_t ibus_sensor_update(ibus_sensor_handle_t handle) {
    ibus_sensor_handle_impl* handle_impl = (ibus_sensor_handle_impl*) handle;
    if (handle_impl->sensor_count == 0 ) {
        return ESP_OK;
    }

    size_t len;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(CONFIG_IBUS_SENSOR_UART_NUM, (size_t*)&len));
    if (len > 0) {
        len = uart_read_bytes(CONFIG_IBUS_SENSOR_UART_NUM, handle_impl->uart_buffer, IBUS_UART_BUFFER_SIZE, 5 / portTICK_RATE_MS);

        return parse_sensor_input(handle_impl);
    }

    return ESP_FAIL;
}

