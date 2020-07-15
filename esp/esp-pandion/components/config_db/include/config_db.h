#pragma once

#include "esp_types.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"

esp_err_t config_db_init(const char* dbName);

esp_err_t config_db_get_float(const char* key, float* value);

esp_err_t config_db_get_int(const char* key, int32_t* value);

float config_db_get_float_def(const char* key, float defaultVal);

int32_t config_db_get_int_def(const char* key, int32_t defaultVal);

esp_err_t config_db_set_float(const char* key, float value);

esp_err_t config_db_set_int(const char* key, int32_t value);