#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_types.h"
#include "driver/timer.h"
#include "driver/periph_ctrl.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "ibus_control.h"
#include "ibus_sensor.h"


#define IBUS_TIMER_DIVIDER 16
#define IBUS_TIMER_GROUP TIMER_GROUP_0
#define IBUS_TIMER_IDX 0
#define IBUS_TIMER_MS_ALARM (TIMER_BASE_CLK / IBUS_TIMER_DIVIDER / 1000)

#define IBUS_UART_QUEUE_SIZE 10
#define IBUS_UART_BUFFER_SIZE 2048
#define IBUS_UART_BAUD_RATE 115200
#define IBUS_MAX_LENGTH 32

#define IBUS_CMD_CONTROL 0x40
#define IBUS_CMD_SENSOR_DISCOVER 0x80
#define IBUS_CMD_SENSOR_TYPE 0x90
#define IBUS_CMD_SENSOR_SEND 0xA0

#define IBUS_UART_DEFAULT_CONFIG() {           \
        .baud_rate = IBUS_UART_BAUD_RATE,      \
        .data_bits = UART_DATA_8_BITS,         \
        .parity = UART_PARITY_DISABLE,         \
        .stop_bits = UART_STOP_BITS_1,         \
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE  \
    };

typedef struct {
    uint64_t timer_counter_value;
} timer_event_t;

esp_err_t ibus_init();

esp_err_t ibus_test_checksum(uint8_t* data);