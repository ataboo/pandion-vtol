#include "flight_control.h"

// `get hnpid_p\n` - Get the 'p' gain for horizontal neutral PID. 
// `set vppid_i 0.123\n` - Set the 'i' gain for the vertical positive PID.

#define TCP_GET_COMMAND "get"
#define TCP_SET_COMMAND "set"
#define TCP_CLEAR_COMMAND "clear"
#define TCP_RESET_COMMAND "reset"
#define TCP_PING_COMMAND "ping"

static const char* TAG = "PANDION_SERVER_COMMANDS";
static nvs_handle_t storage_handle;

static void handle_get_int32(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    int32_t int_val = 0;
    if (nvs_get_i32(storage_handle, packet_in.noun, &int_val) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get %s", packet_in.noun);
    }

    sprintf(packet_out->payload, "%d", int_val);
    packet_out->length = strlen(packet_out->payload);
}

static void handle_set_int32(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    int32_t int_val = atoi(packet_in.payload);

    nvs_set_i32(storage_handle, packet_in.noun, int_val);
    nvs_commit(storage_handle);

    strcpy(packet_out->payload, "ack");
    packet_out->payloadLen = 3;
}

static void handle_get_float(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    char float_buffer[32];
    int32_t int_val = 0;

    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_get_i32(storage_handle, packet_in.noun, &int_val));
    
    float float_val = *(float*)&int_val;

    sprintf(float_buffer, "%f", float_val);
    int float_len = strlen(float_buffer);
    for(int i=0; i<float_len; i++) {
        packet_out->payload[i] = float_buffer[i];
    }

    packet_out->payloadLen = float_len;
}

static void handle_set_float(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    float_t float_val = atof(packet_in.payload);
    int32_t int_val = *(int32_t*)&float_val;

    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_set_i32(storage_handle, packet_in.noun, int_val));
    nvs_commit(storage_handle);

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
    tcp_server_add_handler(TCP_GET_COMMAND, handle_get_float);
    tcp_server_add_handler(TCP_SET_COMMAND, handle_set_float);
    tcp_server_add_handler(TCP_RESET_COMMAND, handle_reset);
    tcp_server_add_handler(TCP_PING_COMMAND, handle_ping);
}

esp_err_t pandion_server_commands_init() {
    register_pandion_server_commands();

    return ESP_OK;
}
