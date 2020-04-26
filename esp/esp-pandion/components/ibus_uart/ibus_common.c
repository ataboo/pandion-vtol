#include "ibus_common.h"

static const char* TAG = "IBUS_COMMON";

uint16_t ibus_calculate_checksum(uint8_t* data) {
    uint8_t len = data[0];
    uint16_t checksum_sum = 0;
    for(int i=0; i<len-2; i++) {
        checksum_sum  += data[i];
    }

    return 0xFFFF - checksum_sum;
}

esp_err_t ibus_test_checksum(uint8_t* data, const char* tag) {
    uint8_t len = data[0];
    if (len <= 3 || len > IBUS_MAX_LENGTH) {
        ESP_LOGW(TAG, "Failed sumcheck length: %d", len);
        return ESP_FAIL;
    }

    uint16_t computed_checksum = ibus_calculate_checksum(data);
    uint16_t payload_checksum = (data[len-1]<<8) | data[len-2];

    if (computed_checksum != payload_checksum) {
        ESP_LOGW(TAG, "%s Failed checksum | computed: %x != payload: %x", tag, computed_checksum, payload_checksum);
        return ESP_FAIL;
    }

    return ESP_OK;
}
