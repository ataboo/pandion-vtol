#pragma once

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

// [length low, length high, command, payload..., checksum low, checksum high]

#define TCP_MAX_COMMAND_LENGTH 256
#define TCP_MAX_HANDLER_COUNT 32
#define CONFIG_TCP_CMD_PORT 3000

typedef struct {
    int length;
    int verbLen;
    int nounLen;
    int payloadLen;
    const char* verb;
    const char* noun;
    uint8_t payload[TCP_MAX_COMMAND_LENGTH-2];
} tcp_command_packet_t;

typedef void (*tcp_handler_t)(tcp_command_packet_t incoming_command, tcp_command_packet_t* outgoing_command);

esp_err_t tcp_server_init();

esp_err_t tcp_server_add_handler(const char* command, tcp_handler_t handler);