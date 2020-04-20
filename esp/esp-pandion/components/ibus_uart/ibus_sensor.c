#include "ibus_sensor.h"

esp_err_t ibus_sensor_update(ibus_sensor_vals_t* channel_vals) {
    int len;

    ESP_ERROR_CHECK(uart_get_buffered_data_len(CONFIG_IBUS_SENSOR_UART_NUM, (size_t*)&len));
    if (len > 0) {

        len = uart_read_bytes(CONFIG_IBUS_SENSOR_UART_NUM, read_buffer, IBUS_UART_BUFFER_SIZE, 5 / portTICK_RATE_MS);

        return parse_channel_values(channel_vals, len);
    }

    return ESP_FAIL;
}

