#include "config_db.h"

static nvs_handle_t nvs_storage_handle;
static bool storage_initialized = false;
static int32_t intBuffer;
static float floatBuffer;

static float int32_to_float(int32_t intVal) {
    return *(float*)&intVal;
}

static int32_t float_to_int32(float floatVal) {
    return *(int32_t*)&floatVal;
}

esp_err_t config_db_init(const char* dbName) {
    if (storage_initialized) {
        return ESP_FAIL;
    }

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open(dbName, NVS_READWRITE, &nvs_storage_handle);
    ESP_ERROR_CHECK(err);

    storage_initialized = true;

    return ESP_OK;
}

esp_err_t config_db_get_float(const char* key, float* value) {
    esp_err_t err = nvs_get_i32(nvs_storage_handle, key, &intBuffer);
    if (err != ESP_OK){
        return err;
    }

    *value = int32_to_float(intBuffer);

    return ESP_OK;
}

esp_err_t config_db_get_int(const char* key, int32_t* value) {
    return nvs_get_i32(nvs_storage_handle, key, value);
}

float config_db_get_float_def(const char* key, float defaultVal) {    
    esp_err_t err = config_db_get_float(key, &floatBuffer);
    if (err != ESP_OK) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(config_db_set_float(key, defaultVal));
        
        return defaultVal;
    }
    
    return floatBuffer;
}

int32_t config_db_get_int_def(const char* key, int32_t defaultVal) {
    esp_err_t err = config_db_get_int(key, &intBuffer);
    if (err != ESP_OK) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(config_db_set_int(key, defaultVal));
        
        return defaultVal;
    }

    return intBuffer;
}

esp_err_t config_db_set_float(const char* key, float value) {
    intBuffer = float_to_int32(value);
    
    return nvs_set_i32(nvs_storage_handle, key, intBuffer);
}

esp_err_t config_db_set_int(const char* key, int32_t value) {
    return nvs_set_i32(nvs_storage_handle, key, value);
}