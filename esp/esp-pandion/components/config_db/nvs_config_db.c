#include "config_db.h"

static nvs_handle_t nvs_storage_handle;
static bool storage_initialized = false;
static int32_t intBuffer;
static float floatBuffer;
static const char* db_name;

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

    db_name = dbName;

    // // Initialize NVS
    // esp_err_t err = nvs_flash_init();
    // if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //     // NVS partition was truncated and needs to be erased
    //     // Retry nvs_flash_init
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     err = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(err);

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
    ESP_LOGE("NVSDB", "Handle: %d", nvs_storage_handle);
    esp_err_t err = nvs_get_i32(nvs_storage_handle, key, value);

    return err;
}

float config_db_get_float_def(const char* key, float defaultVal) {    
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_open(db_name, NVS_READWRITE, &nvs_storage_handle));
    
    esp_err_t err = config_db_get_float(key, &floatBuffer);
    if (err != ESP_OK) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(config_db_set_float(key, defaultVal));
        
        return defaultVal;
    }
    nvs_close(nvs_storage_handle);
    
    return floatBuffer;
}

int32_t config_db_get_int_def(const char* key, int32_t defaultVal) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_open(db_name, NVS_READWRITE, &nvs_storage_handle));

    esp_err_t err = config_db_get_int(key, &intBuffer);
    if (err != ESP_OK) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(config_db_set_int(key, defaultVal));
        
        return defaultVal;
    }
    nvs_close(nvs_storage_handle);

    return intBuffer;
}

esp_err_t config_db_set_float(const char* key, float value) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_open(db_name, NVS_READWRITE, &nvs_storage_handle));
    intBuffer = float_to_int32(value);
    esp_err_t err = nvs_set_i32(nvs_storage_handle, key, intBuffer);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_commit(nvs_storage_handle);
    nvs_close(nvs_storage_handle);

    return err;
}

esp_err_t config_db_set_int(const char* key, int32_t value) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_open(db_name, NVS_READWRITE, &nvs_storage_handle));
    esp_err_t err = nvs_set_i32(nvs_storage_handle, key, value);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_commit(nvs_storage_handle);
    nvs_close(nvs_storage_handle);

    return err;
}