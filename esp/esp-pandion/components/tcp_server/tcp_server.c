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
    tx_buffer[0] = 'p';
    tx_buffer[1] = '|';
    for(int i=2; i<tx_packet_buffer.length; i++) {
        tx_buffer[i] = tx_packet_buffer.payload[i-2];
    }

    send(sock, &tx_buffer, tx_packet_buffer.length, 0);

    ESP_LOGI(TAG, "Sent command");

    return ESP_OK;
}

static esp_err_t handle_command(int sock) {

    ESP_LOGI(TAG, "Handle count now: %d", handler_count);

    for(int i=0; i<handler_count; i++) {
        ESP_LOGI(TAG, "Comparing: %s and %s", handlers[i].verb, rx_packet_buffer.verb);
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

    char verbBuffer[32];
    rx_packet_buffer.verb = verbBuffer;

    for(int i=0; i<len; i++) {
        if (rx_buffer[i] == ' ') {
            verbBuffer[i] = '\0';
            rx_packet_buffer.verbLen = i+1;
            break;
        } else {
            verbBuffer[i] = rx_buffer[i];

            if (i == len-1) {
                verbBuffer[i+1] = '\0';
                rx_packet_buffer.verbLen = i+1;
                break;
            }
        }
    }

    if (len > rx_packet_buffer.verbLen) {
        char nounBuffer[32];
        rx_packet_buffer.noun = nounBuffer;
        for (int i=rx_packet_buffer.verbLen; i<len; i++) {
            if (rx_buffer[i] == ' ') {
                nounBuffer[i] = '\0';
                rx_packet_buffer.nounLen = i+1 - rx_packet_buffer.verbLen;
                break;
            } else {
                nounBuffer[i] = rx_buffer[i];

                if (i == len-1) {
                    nounBuffer[i+1] = '\0';
                    rx_packet_buffer.nounLen = i+1 - rx_packet_buffer.verbLen;
                    break;
                }
            }
        }
    } else {
        rx_packet_buffer.noun = "";
        rx_packet_buffer.nounLen = 0;
    }

    if (len > rx_packet_buffer.verbLen + rx_packet_buffer.nounLen) {
        rx_packet_buffer.payloadLen = len - rx_packet_buffer.verbLen - rx_packet_buffer.nounLen;
        for (int i=rx_packet_buffer.verbLen + rx_packet_buffer.nounLen; i<len; i++) {
            rx_packet_buffer.payload[i] = rx_buffer[i];
        }
    } else {
        rx_packet_buffer.payloadLen = 0;
    }

    ESP_LOGI(TAG, "Parsed verb: %s (%d), noun: %s (%d), payload: (%d)", 
        rx_packet_buffer.verb, 
        rx_packet_buffer.verbLen, 
        rx_packet_buffer.noun, 
        rx_packet_buffer.nounLen,
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
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

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

    ESP_LOGI(TAG, "Handle count now: %d", handler_count);

    return ESP_OK;
}