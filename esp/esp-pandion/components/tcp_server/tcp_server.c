#include "tcp_server.h"

typedef struct {
    const char* verb;
    tcp_handler_t handler;
} registered_handler_t;

static const char *TAG = "TCP_SERVER";
static uint8_t rx_buffer[TCP_MAX_COMMAND_LENGTH];
static uint8_t tx_buffer[TCP_MAX_COMMAND_LENGTH];
static tcp_command_packet_t rx_packet_buffer;
static tcp_command_packet_t tx_packet_buffer;
static registered_handler_t handlers[TCP_MAX_HANDLER_COUNT];
static int handler_count = 0;


static esp_err_t send_command(int sock) {
    if (tx_packet_buffer.payloadLen == 0) {
        tx_packet_buffer.payloadLen = 4;
        strcpy(tx_packet_buffer.payload, "ack");
    }

    tx_packet_buffer.length = tx_packet_buffer.payloadLen;
    
    memcpy(tx_buffer, tx_packet_buffer.payload, tx_packet_buffer.payloadLen);

    send(sock, &tx_buffer, tx_packet_buffer.length, 0);

    return ESP_OK;
}

static esp_err_t handle_command(int sock) {
    for(int i=0; i<handler_count; i++) {
        if (strcmp(handlers[i].verb, rx_packet_buffer.verb) == 0) {
            handlers[i].handler(rx_packet_buffer, &tx_packet_buffer);
            return send_command(sock);
        }
    }

    ESP_LOGW(TAG, "No handler found for command: `%s`", rx_packet_buffer.verb);
    return ESP_FAIL;
}

static esp_err_t parse_command(int len, int sock) {
    if (len < 2) {
        ESP_LOGE(TAG, "Packet of length %d is too short.", len);
        return ESP_FAIL;
    }

    if (len > TCP_MAX_COMMAND_LENGTH) {
        ESP_LOGE(TAG, "Packet of length %d is too long.", len);
        return ESP_FAIL;
    }

    rx_packet_buffer.length = len;

    char* token = strtok((char*)rx_buffer, " ");

    strcpy(rx_packet_buffer.verb, token);
    rx_packet_buffer.verbLen = strlen(rx_packet_buffer.verb);

    token = strtok(NULL, " ");
    if (token != NULL) {
        rx_packet_buffer.nounLen = strlen(token);
        strcpy(rx_packet_buffer.noun, token);
    } else {
        rx_packet_buffer.nounLen = 0;
    }

    if (rx_packet_buffer.nounLen > 0) {
        token = strtok(NULL, " ");

        if (token != NULL) {
            rx_packet_buffer.payloadLen = strlen(token);
            strcpy(rx_packet_buffer.payload, token);
        } else {
            rx_packet_buffer.payloadLen = 0;
        }
    }

    ESP_LOGD(TAG, "Parsed verb: %s (%d), noun: %s (%d), payload: %s (%d)", 
        rx_packet_buffer.verb, 
        rx_packet_buffer.verbLen, 
        rx_packet_buffer.noun, 
        rx_packet_buffer.nounLen,
        rx_packet_buffer.payload,
        rx_packet_buffer.payloadLen
    );

    handle_command(sock);

    return ESP_OK;
}

static esp_err_t read_command(int sock) {
    int len;
    
    do {
        len = recv(sock, rx_buffer, TCP_MAX_COMMAND_LENGTH, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0;

            parse_command(len, sock);
        }
    } while (len > 0);

    return ESP_OK;
}

static esp_err_t init_server_socket(struct sockaddr* dest_addr, int* listen_sock) {
    
    *listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (*listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Socket created");

    int err = bind(*listen_sock, dest_addr, sizeof(*dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", CONFIG_TCP_CMD_PORT);

    err = listen(*listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void tcp_server_task()
{
    char addr_str[128];
    struct sockaddr_in dest_addr;
    int listen_sock = 0;

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(CONFIG_TCP_CMD_PORT);

    init_server_socket((struct sockaddr*)&dest_addr, &listen_sock);

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        inet_ntoa_r((source_addr).sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        read_command(sock);

        shutdown(sock, 0);
        close(sock);
    }

    close(listen_sock);
    vTaskDelete(NULL);
}

esp_err_t tcp_server_init() {
    rx_packet_buffer = (tcp_command_packet_t){ };
    tx_packet_buffer = (tcp_command_packet_t){ };

    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);

    return ESP_OK;
}

esp_err_t tcp_server_add_handler(const char* verb, tcp_handler_t handler) {
    if (handler_count == TCP_MAX_HANDLER_COUNT) {
        ESP_LOGE(TAG, "Max number of handlers reached.");
        return ESP_FAIL;
    }

    registered_handler_t new_handler = {
        .handler = handler,
        .verb = verb
    };

    handlers[handler_count++] = new_handler;

    return ESP_OK;
}