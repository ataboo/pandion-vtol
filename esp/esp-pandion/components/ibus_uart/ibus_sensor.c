#if CONFIG_IBUS_UART_TX_ACTIVE
static void ibus_init_tx_uart(ibus_rx_handle_t handle) {
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