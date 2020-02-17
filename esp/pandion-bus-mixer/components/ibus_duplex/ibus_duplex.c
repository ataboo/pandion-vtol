#include "ibus_duplex.h"

#define IBUS_UART_BUFFER_SIZE 2048
#define IBUS_UART_QUEUE_SIZE 10

struct ibus_duplex_handle_impl {
    ibus_channel_vals_t values;
    TaskHandle_t task_handle;
    QueueHandle_t rx_queue_handle;
};

static void ibus_init_rx_uart(ibus_duplex_handle_t handle) {
    uart_config_t uart_config = {
        .baud_rate = CONFIG_IBUS_UART_RX_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(CONFIG_IBUS_UART_RX_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(CONFIG_IBUS_UART_RX_NUM, UART_PIN_NO_CHANGE, CONFIG_IBUS_UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_driver_install(CONFIG_IBUS_UART_RX_NUM, IBUS_UART_BUFFER_SIZE, IBUS_UART_BUFFER_SIZE, IBUS_UART_QUEUE_SIZE, &handle->rx_queue_handle, 0));
}

#if CONFIG_IBUS_UART_TX_ACTIVE
static void ibus_init_tx_uart(ibus_duplex_handle_t handle) {
    // TODO
}
#endif

ibus_duplex_handle_t ibus_duplex_init() {
    ibus_duplex_handle_t ibus_handle = (ibus_duplex_handle_t)malloc(sizeof(ibus_duplex_handle_impl));

    ibus_init_rx_uart(ibus_handle);

#if CONFIG_IBUS_UART_TX_ACTIVE
    ibus_init_tx_uart(ibus_handle);
#endif

    // Register uart interrupts

    return ibus_handle;
}

void ibus_duplex_terminate(ibus_duplex_handle_t handle) {
    // TODO

    // Stop task, close uarts.
}

uint16_t ibus_get_channel_value(ibus_duplex_handle_t ibus_handle) {
    //TODO
    
    return 0;
}

esp_err_t ibus_get_channel_values(ibus_duplex_handle_t ibus_handle, ibus_channel_vals_t* channel_vals) {
    //TODO
    
    return ESP_OK;
}