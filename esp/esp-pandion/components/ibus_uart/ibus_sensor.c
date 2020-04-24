#include "ibus_sensor.h"

static const char* TAG = "IBUS_SENSOR";

typedef struct {
    int sensor_count;
    QueueHandle_t uart_queue;
    uint8_t read_buffer[IBUS_UART_BUFFER_SIZE];
    uint8_t write_buffer[IBUS_UART_BUFFER_SIZE];
    ibus_sensor_t* sensors[IBUS_MAX_SENSOR_COUNT];
} ibus_sensor_handle_impl;

ibus_sensor_handle_t ibus_sensor_init(uart_port_t uart_num, gpio_num_t rx_gpio, gpio_num_t tx_gpio) {
    ibus_sensor_handle_impl* handle_impl = malloc(sizeof(ibus_sensor_handle_impl));
    handle_impl->sensor_count = 0;

    uart_config_t uart_config = IBUS_UART_DEFAULT_CONFIG();
    
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_set_pin(uart_num, tx_gpio, rx_gpio, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_driver_install(uart_num, IBUS_UART_BUFFER_SIZE, IBUS_UART_BUFFER_SIZE, IBUS_UART_QUEUE_SIZE, &handle_impl->uart_queue, 0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_flush_input(uart_num));

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

    handle_impl->sensors[handle_impl->sensor_count++] = sensor;

    return ESP_OK;
}

esp_err_t parse_sensor_input(ibus_sensor_handle_impl* handle_impl) {
    uint8_t len = handle_impl->read_buffer[0];
    if (len != 4) {
        // ESP_LOGW(TAG, "Length of %d invalid for IBus sensor.", len);
        return ESP_FAIL;
    }
    
    esp_err_t ret = ibus_test_checksum(handle_impl->read_buffer, TAG);
    if (ret != ESP_OK) {
        return ret;
    }

    // ESP_LOGI(TAG, "%.2x, %.2x, %.2x, %.2x", handle_impl->uart_buffer[0], handle_impl->uart_buffer[1], handle_impl->uart_buffer[2], handle_impl->uart_buffer[3]);

    uint8_t addr = handle_impl->read_buffer[1] & 0x0F;
    uint8_t command = handle_impl->read_buffer[1] & 0xF0;

    if (addr > handle_impl->sensor_count) {
        return ESP_FAIL;
    }

    switch(command) {
        case IBUS_CMD_SENSOR_DISCOVER:
            // ESP_LOGI(TAG, "Received sensor discover for %d", addr);
            len = 0x04;
            handle_impl->write_buffer[1] = command + addr;
            break;
        case IBUS_CMD_SENSOR_TYPE:
            // ESP_LOGI(TAG, "Received sensor type for %d", addr);
            len = 0x06;
            handle_impl->write_buffer[1] = command + addr;
            handle_impl->write_buffer[2] = handle_impl->sensors[addr-1]->sensor_type;
            handle_impl->write_buffer[3] = 0x02;
            break;
        case IBUS_CMD_SENSOR_SEND:
            // ESP_LOGI(TAG, "Received sensor send for %d", addr);
            len = 0x06;
            handle_impl->write_buffer[1] = command + addr;
            handle_impl->write_buffer[2] = handle_impl->sensors[addr-1]->value & 0xFF;
            handle_impl->write_buffer[3] = handle_impl->sensors[addr-1]->value >> 8;
            break;
        default:
            ESP_LOGW(TAG, "Unsupported IBus sensor command: %x", handle_impl->read_buffer[1]);
            return ESP_FAIL;
    }

    handle_impl->write_buffer[0] = len;
    uint16_t check_sum = ibus_calculate_checksum(handle_impl->write_buffer);
    handle_impl->write_buffer[len-2] = check_sum & 0xFF;
    handle_impl->write_buffer[len-1] = (check_sum >> 8);

    int n = uart_write_bytes(CONFIG_IBUS_SENSOR_UART_NUM, (char*)handle_impl->write_buffer, len);
    uart_wait_tx_done(CONFIG_IBUS_SENSOR_UART_NUM, 2 / portMAX_DELAY);
    uart_flush(CONFIG_IBUS_SENSOR_UART_NUM);
    if (n == len) {
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to send UART");
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
        len = uart_read_bytes(CONFIG_IBUS_SENSOR_UART_NUM, handle_impl->read_buffer, IBUS_UART_BUFFER_SIZE, 2 / portTICK_RATE_MS);

        return parse_sensor_input(handle_impl);
    }

    return ESP_FAIL;
}

