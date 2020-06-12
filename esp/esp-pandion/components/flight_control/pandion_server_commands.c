#include "flight_control.h"

#define TCP_COMMAND_PITCH_SET_P 0x0110
#define TCP_COMMAND_PITCH_SET_I 0x0111
#define TCP_COMMAND_PITCH_SET_D 0x0112
#define TCP_COMMAND_PITCH_GET_P 0x0113
#define TCP_COMMAND_PITCH_GET_I 0x0114
#define TCP_COMMAND_PITCH_GET_D 0x0115
#define TCP_COMMAND_ROLL_SET_P 0x0116
#define TCP_COMMAND_ROLL_SET_I 0x0117
#define TCP_COMMAND_ROLL_SET_D 0x0118
#define TCP_COMMAND_ROLL_GET_P 0x0119
#define TCP_COMMAND_ROLL_GET_I 0x011a
#define TCP_COMMAND_ROLL_GET_D 0x011b
#define TCP_COMMAND_YAW_SET_P 0x011c
#define TCP_COMMAND_YAW_SET_I 0x011d
#define TCP_COMMAND_YAW_SET_D 0x011e
#define TCP_COMMAND_YAW_GET_P 0x011f
#define TCP_COMMAND_YAW_GET_I 0x0120
#define TCP_COMMAND_YAW_GET_D 0x0121

static const char* TAG = "PANDION_SERVER_COMMANDS";
static nvs_handle_t nvs_handle;

static void handle_get_int32(const char* key, tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    uint32_t int_val = 0;
    if (nvs_get_i32(nvs_handle, key, &int_val) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get pandiond server command %s", key);
    }

    packet_out->length = 4;
    memcpy(packet_out->payload, int_val);
}

static void handle_set_int32(const char* key, tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    if (packet_in.length != 6) {
        ESP_LOGE(TAG, "Unnexpected tcp command packet length, %d", packet_in.length);
    }
    
    int32_t int_val = 0;
    memcpy(int_val, packet_in.payload, 4);
    
    nvs_set_i32(nvs_handle, key, int_val);
} 

static void 

static void handle_get_pitch_p(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    handle_get_int32("pandion_pitch_p", packet_in, packet_out);
}

static void handle_set_pitch_p(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    handle_set_int32("pandion_pitch_p", packet_in, packet_out);
}

static void handle_get_pitch_i(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    handle_get_int32("pandion_pitch_i", packet_in, packet_out);
}

static void handle_set_pitch_i(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    handle_set_int32("pandion_pitch_i", packet_in, packet_out);
}

static void handle_get_pitch_d(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    handle_get_int32("pandion_pitch_d", packet_in, packet_out);
}

static void handle_set_pitch_d(tcp_command_packet_t packet_in, tcp_command_packet_t* packet_out) {
    handle_set_int32("pandion_pitch_d", packet_in, packet_out);
}

esp_err_t pandion_server_commands_init() {
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
}

void register_pandion_server_commands() {
    tcp_server_add_handler(TCP_COMMAND_PITCH_GET_P, handle_get_pitch_p);
    tcp_server_add_handler(TCP_COMMAND_PITCH_SET_P, handle_set_pitch_p);
    tcp_server_add_handler(TCP_COMMAND_PITCH_GET_I, handle_get_pitch_i);
    tcp_server_add_handler(TCP_COMMAND_PITCH_SET_I, handle_set_pitch_i);
    tcp_server_add_handler(TCP_COMMAND_PITCH_GET_D, handle_get_pitch_d);
    tcp_server_add_handler(TCP_COMMAND_PITCH_SET_D, handle_set_pitch_d);
}