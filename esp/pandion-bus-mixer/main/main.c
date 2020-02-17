#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "ibus_duplex.h"

#define IBUS_RXD 16
#define IBUS_UART_NUM UART_NUM_2

static const char* TAG = "PANDION_BUS_MIXER";

void app_main()
{
    // const int uart_num = UART_NUM_2;
    // uart_config_t uart_config = {
    //     .baud_rate = 115200,
    //     .data_bits = UART_DATA_8_BITS,
    //     .parity = UART_PARITY_DISABLE,
    //     .stop_bits = UART_STOP_BITS_1,
    //     .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    //     .rx_flow_ctrl_thresh = 122,
    // };
    
    // // Configure UART parameters
    // ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    // ESP_ERROR_CHECK(uart_set_pin(IBUS_UART_NUM, 17, 16, 18, 19));

    // UART_PIN_NO_CHANGE

    // const int uart_buffer_size = (1024 * 2);
    // QueueHandle_t uart_queue;
    // ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

    uint8_t data[128];
    int length = 0;
    char print_buffer[1024];
    char short_buff[256];
    uint16_t int_vals[16];

    while(1) {
        ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t*)&length));
        length = uart_read_bytes(uart_num, data, length, 100);

        if (length >= 32) {
            for(int i=0; i<32; i+=2) {
                int_vals[i/2] = data[i] | (uint16_t)data[i+1] << 8;

                // sprintf(short_buff, "%d, ", int_vals[i/2]);
                // strcat(print_buffer, short_buff);
            }

            ESP_LOGI(TAG, "Vals: %d, %d, %d, %d, %d, %d", int_vals[0], int_vals[1], int_vals[2], int_vals[3], int_vals[4], int_vals[5]);
            // ESP_LOGI(TAG, "Values: %s", print_buffer);
        }

        vTaskDelay(5/portTICK_RATE_MS);
    }
}
