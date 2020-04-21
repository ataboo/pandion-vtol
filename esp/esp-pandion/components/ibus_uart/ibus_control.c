#include "ibus_control.h"

static const char* TAG = "IBUS_CTRL";

static uint8_t read_buffer[IBUS_UART_BUFFER_SIZE];

static esp_err_t parse_channel_values(ibus_ctrl_channel_vals_t* channel_vals, int len) {
    if(len != 32) {
        ESP_LOGW(TAG, "Unnexpected IBus command length %d.", len);
        return ESP_FAIL;
    }

    if (read_buffer[1] != IBUS_CMD_CONTROL) {
        ESP_LOGW(TAG, "Unnexpected first char %x in ibus control command.", read_buffer[1]);
        return ESP_FAIL;
    }

    esp_err_t ret = ibus_test_checksum(read_buffer, TAG);
    if (ret != ESP_OK) {
        return ret;
    }

    for(int i=0; i<28; i+=2) {
        channel_vals->channels[i/2] = read_buffer[i+2] | (uint16_t)read_buffer[i+3] << 8;
    }

    return ESP_OK;
}

esp_err_t ibus_control_init() {
    uart_config_t uart_config = IBUS_UART_DEFAULT_CONFIG();
    
    esp_err_t ret = uart_param_config(CONFIG_IBUS_CTRL_UART_NUM, &uart_config);
    ret |= uart_set_pin(CONFIG_IBUS_CTRL_UART_NUM, CONFIG_IBUS_CTRL_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ret |= uart_driver_install(uart_num, IBUS_UART_BUFFER_SIZE, IBUS_UART_BUFFER_SIZE, IBUS_UART_QUEUE_SIZE, uart_queue_handle, 0);
    ret |= uart_flush_input(CONFIG_IBUS_CTRL_UART_NUM);

    return ret;
}

esp_err_t ibus_control_update(ibus_ctrl_channel_vals_t* channel_vals) {
    int len;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(CONFIG_IBUS_CTRL_UART_NUM, (size_t*)&len));
    if (len > 0) {
        len = uart_read_bytes(CONFIG_IBUS_CTRL_UART_NUM, read_buffer, IBUS_UART_BUFFER_SIZE, 1 / portTICK_RATE_MS);

        return parse_channel_values(channel_vals, len);
    }

    return ESP_FAIL;
}
