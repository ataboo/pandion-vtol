#include "tcp_server.h"

typedef struct {
    uint16_t command;
    tcp_handler_t handler;
} registered_handler_t;

static const char *TAG = "TCP_SERVER";
static uint8_t rx_buffer[TCP_MAX_COMMAND_LENGTH];
static tcp_command_packet_t rx_packet_buffer;
static tcp_command_packet_t tx_packet_buffer;
static registered_handler_t handlers[TCP_MAX_HANDLER_COUNT];
static int handler_count;

static esp_err_t handle_command() {
    bool found_handler = false;

    for(int i=0; i<handler_count; i++) {
        if (handlers[i].command == rx_packet_buffer.command) {
            tx_packet_buffer.command = -1;
            handlers[i].handler(rx_packet_buffer, &tx_packet_buffer);
            if (tx_packet_buffer.command > -1 ) {
                send_command();
            }
            
            found_handler = true;
        }
    }

    if (!found_handler) {
        ESP_LOGW(TAG, "No handler found for command: %d", rx_packet_buffer.command);
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

static esp_err_t send_command() {

    // send() can return less bytes than supplied length.
    // Walk-around for robust implementation. 
    // int to_write = len;
    // while (to_write > 0) {
    //     int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
    //     if (written < 0) {
    //         ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    //     }
    //     to_write -= written;
    // }

    return ESP_OK;
}

static esp_err_t parse_command(int len) {
    if (len < 2) {
        ESP_LOGE(TAG, "Packet of length %d is too short.", len);
        return ESP_FAIL;
    }

    rx_packet_buffer.command = rx_buffer[0] | rx_buffer[1] << 8;
    
    for (int i=2; i<len; i++) {
        rx_packet_buffer.payload[i] = rx_buffer[i];
    }

    handle_command();

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
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            parse_command(len);
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
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);

    return ESP_OK;
}

esp_err_t tcp_server_add_handler(uint16_t command, tcp_handler_t handler) {
    if (handler_count == TCP_MAX_HANDLER_COUNT) {
        ESP_LOGE(TAG, "Max number of handlers reached.");
        return ESP_FAIL;
    }

    registered_handler_t new_handler = {
        .handler = handler,
        .command = command
    };

    handlers[handler_count++] = new_handler;

    return ESP_OK;
}