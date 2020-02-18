#include "ibus_duplex.h"

#define IBUS_UART_BUFFER_SIZE 2048
#define IBUS_UART_QUEUE_SIZE 10
#define IBUS_UART_READ_BUFFER_SIZE 1024

static const char* TAG = "IBUS_DUPLEX";

struct ibus_duplex_handle_impl {
    ibus_channel_vals_t values;
    TaskHandle_t task_handle;
    QueueHandle_t rx_queue_handle;
    QueueHandle_t tx_queue_handle;
};

static void ibus_init_rx_uart(ibus_duplex_handle_t handle) {
    uart_config_t uart_config = {
        .baud_rate = CONFIG_IBUS_UART_RX_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    ESP_ERROR_CHECK(uart_param_config(CONFIG_IBUS_UART_RX_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CONFIG_IBUS_UART_RX_NUM, UART_PIN_NO_CHANGE, CONFIG_IBUS_UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_IBUS_UART_RX_NUM, IBUS_UART_BUFFER_SIZE, IBUS_UART_BUFFER_SIZE * 2, IBUS_UART_QUEUE_SIZE, &handle->rx_queue_handle, 0));
}

#if CONFIG_IBUS_UART_TX_ACTIVE
static void ibus_init_tx_uart(ibus_duplex_handle_t handle) {
    uart_config_t uart_config = {
        .baud_rate = CONFIG_IBUS_UART_TX_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    ESP_ERROR_CHECK(uart_param_config(CONFIG_IBUS_UART_TX_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CONFIG_IBUS_UART_TX_NUM, CONFIG_IBUS_UART_TX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_IBUS_UART_TX_NUM, IBUS_UART_BUFFER_SIZE, IBUS_UART_BUFFER_SIZE, IBUS_UART_QUEUE_SIZE, &handle->tx_queue_handle, 0));
}
#endif

static esp_err_t parse_channel_values(ibus_duplex_handle_t handle, uint8_t* data, int len) {
    if(len < 32) {
        ESP_LOGE(TAG, "Cannot parse channel values as buffer is too short %d.", len);
        return ESP_FAIL;
    }

#if CONFIG_IBUS_UART_SUMCHK_ACTIVE
    uint8_t sum = 0;
    
    for(int i=1; i<31; i++) {
        sum += data[i];
    }

    if (sum != data[31]) {
        ESP_LOGE(TAG, "IBus Sum Check failed");
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "IBus Sum Check passed");
    }
#endif

    handle->values.preamble = data[0] | (uint16_t)data[1] << 8;

    for(int i=0; i<28; i+=2) {
        handle->values.channels[i/2] = data[i+2] | (uint16_t)data[i+3] << 8;
    }

    handle->values.chk_sum = data[30] | (uint16_t)data[31] << 8;

    ESP_LOG_BUFFER_HEX_LEVEL(TAG, data, 32, ESP_LOG_VERBOSE);

    return ESP_OK;
}

static void echo_task(void *arg) {
    ibus_duplex_handle_t ibus_handle = (ibus_duplex_handle_t)arg;
    int len;
    uint8_t *data_buffer = (uint8_t *) malloc(IBUS_UART_READ_BUFFER_SIZE);

    while(1) {
        ESP_ERROR_CHECK(uart_get_buffered_data_len(CONFIG_IBUS_UART_RX_NUM, (size_t*)&len));
        if (len > 0) {
            len = uart_read_bytes(CONFIG_IBUS_UART_RX_NUM, data_buffer, IBUS_UART_READ_BUFFER_SIZE, 20 / portTICK_RATE_MS);
#if CONFIG_IBUS_UART_TX_ACTIVE
            uart_write_bytes(CONFIG_IBUS_UART_TX_NUM, (const char *) data_buffer, len);
#endif
            parse_channel_values(ibus_handle, data_buffer, len);
        }

        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

ibus_duplex_handle_t ibus_duplex_init() {
    ibus_duplex_handle_t ibus_handle = (ibus_duplex_handle_t)malloc(sizeof(struct ibus_duplex_handle_impl));

    ibus_init_rx_uart(ibus_handle);
#if CONFIG_IBUS_UART_TX_ACTIVE
    ibus_init_tx_uart(ibus_handle);
#endif

    xTaskCreate(echo_task, "ibus_duplex_echo", 2048, ibus_handle, configMAX_PRIORITIES, &ibus_handle->task_handle);

    return ibus_handle;
}

void ibus_duplex_terminate(ibus_duplex_handle_t handle) {
    vTaskDelete(handle->task_handle);

    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_driver_delete(CONFIG_IBUS_UART_RX_NUM));
#if CONFIG_IBUS_UART_TX_ACTIVE
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_driver_delete(CONFIG_IBUS_UART_TX_NUM));
#endif
}

uint16_t ibus_get_channel_value(ibus_duplex_handle_t ibus_handle, int channel) {
    return ibus_handle->values.channels[channel];
}

esp_err_t ibus_get_channel_values(ibus_duplex_handle_t ibus_handle, ibus_channel_vals_t* channel_vals) {
    *channel_vals = ibus_handle->values;
    
    return ESP_OK;
}