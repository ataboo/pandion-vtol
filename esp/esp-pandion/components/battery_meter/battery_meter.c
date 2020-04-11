#include "battery_meter.h"                                  

static esp_adc_cal_characteristics_t* adc_chars;
static uint64_t mean_sum;
static uint32_t mean_mv;
static uint32_t current_mv;

static const char* TAG = "BATTERY_METER";

static void read_voltage_level() {
    esp_adc_cal_get_voltage(ADC_CHANNEL_0, adc_chars, &current_mv);
}

esp_err_t battery_meter_init() {
    adc_chars = (esp_adc_cal_characteristics_t*)malloc(sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_12Bit, BATTERY_LEVEL_VREF_MV, adc_chars);

    esp_err_t ret = adc1_config_width(ADC_WIDTH_12Bit);
    ret |= adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_11);

    battery_meter_update();
    mean_mv = current_mv;

    return ret;
}

void battery_meter_update() {
    read_voltage_level();

    mean_sum = mean_mv * (BATTERY_LEVEL_MEAN_SAMPLE_COUNT - 1) + current_mv;
    mean_mv = mean_sum / BATTERY_LEVEL_MEAN_SAMPLE_COUNT;

    ESP_LOGD(TAG, "Battery: %d, %d, %d, %d", current_mv, mean_mv, battery_meter_mv(), battery_meter_state());
}

uint32_t battery_meter_mv() {
    return mean_mv / BATTERY_GPIO_VOLTAGE_FACTOR;
}

battery_state_t battery_meter_state() {
    if(mean_mv < BATTERY_LEVEL_EMPTY_MV) {
        return BATTERY_EMPTY;
    }

    if (mean_mv < BATTERY_LEVEL_ALARM_MV) {
        return BATTERY_ALARM;
    }

    if (mean_mv < BATTERY_LEVEL_FULL_MV) {
        return BATTERY_NOMINAL;
    }

    return BATTERY_OVER;
}
