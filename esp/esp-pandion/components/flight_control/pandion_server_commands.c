#include "flight_control.h"

// `get hnpid_p\n` - Get the 'p' gain for horizontal neutral PID. 
// `set vppid_i 0.123\n` - Set the 'i' gain for the vertical positive PID.

#define TCP_GET_COMMAND "get"
#define TCP_SET_COMMAND "set"
#define TCP_RESET_COMMAND "reset"

#define TCP_VAL_HORIZ_NEUTRAL_PID "hnpid"
#define TCP_VAL_VERT_NEUTRAL_PID "vnpid"

#define TCP_VAL_HORIZ_POSITIVE_PID "hppid"
#define TCP_VAL_VERT_POSITIVE_PID "vppid"


static const char* TAG = "PANDION_SERVER_COMMANDS";
static nvs_handle_t storage_handle;

static void handle_get_int32(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    int32_t int_val = 0;
    if (nvs_get_i32(storage_handle, packet_in.noun, &int_val) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get %s", packet_in.noun);
    }

    packet_out->payload[0] = (int_val>>24) & 0xFF;
    packet_out->payload[1] = (int_val>>16) & 0xFF;
    packet_out->payload[2] = (int_val>>8) & 0xFF;
    packet_out->payload[3] = int_val & 0xFF;
    packet_out->length = 4;
}

static void handle_set_int32(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    if (packet_in.payloadLen != 4) {
        ESP_LOGE(TAG, "Unnexpected tcp command packet length, %d", packet_in.payloadLen);
    }
    
    int32_t int_val = 0;

    int_val = packet_in.payload[0] | packet_in.payload[1] << 8 | packet_in.payload[2] << 16 | packet_in.payload[3] << 24;
    
    nvs_set_i32(storage_handle, packet_in.noun, int_val);
}

static void handle_reset(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    //TODO;
}

static void register_pandion_server_commands() {
    tcp_server_add_handler(TCP_GET_COMMAND, handle_get_int32);
    tcp_server_add_handler(TCP_SET_COMMAND, handle_set_int32);
    tcp_server_add_handler(TCP_RESET_COMMAND, handle_reset);
}

esp_err_t pandion_server_commands_init() {
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &storage_handle);
    if (err != ESP_OK) {
        return err;
    }

    register_pandion_server_commands();

    return ESP_OK;
}

