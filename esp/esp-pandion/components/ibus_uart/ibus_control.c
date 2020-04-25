#include "ibus_control.h"

typedef struct {
    uint8_t read_buffer[IBUS_UART_BUFFER_SIZE];
    ibus_ctrl_channel_vals_t* channel_vals;
    QueueHandle_t uart_queue;
} ibus_ctrl_handle_impl;

static const char* TAG = "IBUS_CTRL";

static esp_err_t parse_channel_values(ibus_ctrl_handle_impl* handle_impl, int len) {
    if(len != 32) {
        ESP_LOGW(TAG, "Unnexpected IBus command length %d.", len);
        return ESP_FAIL;
    }

    if (handle_impl->read_buffer[1] != IBUS_CMD_CONTROL) {
        ESP_LOGW(TAG, "Unnexpected first char %x in ibus control command.", handle_impl->read_buffer[1]);
        return ESP_FAIL;
    }

    esp_err_t ret = ibus_test_checksum(handle_impl->read_buffer, TAG);
    if (ret != ESP_OK) {
        return ret;
    }

    for(int i=0; i<IBUS_RX_CHAN_COUNT * 2; i+=2) {
        handle_impl->channel_vals->channels[i/2] = handle_impl->read_buffer[i+2] | (uint16_t)handle_impl->read_buffer[i+3] << 8;
    }

    return ESP_OK;
}

ibus_ctrl_channel_vals_t* ibus_channel_vals_init() {
    ibus_ctrl_channel_vals_t* channel_vals = malloc(sizeof(ibus_ctrl_channel_vals_t));
    for(int i=0; i<IBUS_RX_CHAN_COUNT; i++) {
        channel_vals->channels[i] = 1500;
    }

    return channel_vals;
}

ibus_ctrl_handle_t ibus_control_init(uart_port_t uart_num, gpio_num_t rx_gpio_pin) {
    ibus_ctrl_handle_impl* handle_impl = malloc(sizeof(ibus_ctrl_handle_impl));
    handle_impl->channel_vals = ibus_channel_vals_init();

    uart_config_t uart_config = IBUS_UART_DEFAULT_CONFIG();
    
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_set_pin(uart_num, UART_PIN_NO_CHANGE, rx_gpio_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_driver_install(uart_num, IBUS_UART_BUFFER_SIZE, IBUS_UART_BUFFER_SIZE, IBUS_UART_QUEUE_SIZE, &handle_impl->uart_queue, 0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_flush_input(uart_num));

    return (ibus_ctrl_handle_t)handle_impl;
}

esp_err_t ibus_control_update(ibus_ctrl_handle_t handle) {
    ibus_ctrl_handle_impl* handle_impl = (ibus_ctrl_handle_impl*)handle;
    
    int len;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(CONFIG_IBUS_CTRL_UART_NUM, (size_t*)&len));
    if (len > 0) {
        len = uart_read_bytes(CONFIG_IBUS_CTRL_UART_NUM, (uint8_t*)handle_impl->read_buffer, IBUS_UART_BUFFER_SIZE, 1 / portTICK_RATE_MS);

        return parse_channel_values(handle_impl, len);
    }

    return ESP_FAIL;
}

void ibus_control_channel_values(ibus_ctrl_handle_t handle, ibus_ctrl_channel_vals_t* channel_vals) {
    ibus_ctrl_handle_impl* handle_impl = (ibus_ctrl_handle_impl*) handle;

    for(int i=0; i<IBUS_RX_CHAN_COUNT; i++) {
        channel_vals->channels[i] = handle_impl->channel_vals->channels[i];
    }
}
