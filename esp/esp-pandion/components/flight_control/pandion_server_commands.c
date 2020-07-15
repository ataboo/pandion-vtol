#include "flight_control.h"

// `get hnpid_p\n` - Get the 'p' gain for horizontal neutral PID. 
// `set vppid_i 0.123\n` - Set the 'i' gain for the vertical positive PID.

#define TCP_GETI_COMMAND "geti"
#define TCP_SETI_COMMAND "seti"
#define TCP_GETF_COMMAND "getf"
#define TCP_SETF_COMMAND "setf"
#define TCP_CLEAR_COMMAND "clear"
#define TCP_RESET_COMMAND "reset"
#define TCP_PING_COMMAND "ping"

static const char* TAG = "PANDION_SERVER_COMMANDS";
static nvs_handle_t storage_handle;

static void handle_get_int32(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    int32_t int_val = config_db_get_int_def(packet_in.noun, 0);

    ESP_LOGI(TAG, "got in: %d", int_val);

    sprintf(packet_out->payload, "%d", int_val);
    packet_out->payloadLen = strlen(packet_out->payload);
}

static void handle_set_int32(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    int32_t int_val = atoi(packet_in.payload);
    config_db_set_int(packet_in.noun, int_val);
    strcpy(packet_out->payload, "ack");
    packet_out->payloadLen = 3;
}

static void handle_get_float(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    char float_buffer[32];
    int32_t int_val = 0;

    float float_val = config_db_get_float_def(packet_in.noun, 0);

    sprintf(float_buffer, "%f", float_val);
    int float_len = strlen(float_buffer);
    for(int i=0; i<float_len; i++) {
        packet_out->payload[i] = float_buffer[i];
    }

    packet_out->payloadLen = float_len;
}

static void handle_set_float(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    float_t float_val = atof(packet_in.payload);
    
    ESP_ERROR_CHECK_WITHOUT_ABORT(config_db_set_float(packet_in.noun, float_val));

    strcpy(packet_out->payload, "ack");
    packet_out->payloadLen = 4;
}

static void handle_reset(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    ESP_LOGI(TAG, "Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

static void handle_ping(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    strcpy(packet_out->payload, "pong");
    packet_out->payloadLen = 5;
}

static void register_pandion_server_commands() {
    tcp_server_add_handler(TCP_GETI_COMMAND, handle_get_int32);
    tcp_server_add_handler(TCP_SETI_COMMAND, handle_set_int32);
    tcp_server_add_handler(TCP_GETF_COMMAND, handle_get_float);
    tcp_server_add_handler(TCP_SETF_COMMAND, handle_set_float);
    tcp_server_add_handler(TCP_RESET_COMMAND, handle_reset);
    tcp_server_add_handler(TCP_PING_COMMAND, handle_ping);
}

esp_err_t pandion_server_commands_init() {
    register_pandion_server_commands();

    return ESP_OK;
}
