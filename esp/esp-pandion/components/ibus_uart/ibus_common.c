#include "ibus_common.h"

static const char* TAG = "IBUS_COMMON";

static xQueueHandle timer_queue = NULL;
static QueueHandle_t control_uart_handle;
static QueueHandle_t sensor_uart_handle;

static ibus_sensor_handle_t sensor_handle;
static ibus_sensor_t extv_sensor;

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
        ESP_LOGW(TAG, "%.2x, %.2x, %.2x, %.2x, %.2x, %.2x", data[0], data[1], data[2], data[3], data[4], data[5]);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void IRAM_ATTR timer_tick_isr(void *para) 
{
    timer_spinlock_take(IBUS_TIMER_GROUP);
    
    uint64_t timer_counter_value = timer_group_get_counter_value_in_isr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);

    timer_event_t evt;
    evt.timer_counter_value = timer_counter_value;

    timer_group_clr_intr_status_in_isr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);
    timer_group_enable_alarm_in_isr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);

    xQueueSendFromISR(timer_queue, &evt, NULL);
    timer_spinlock_give(IBUS_TIMER_GROUP);
}

static esp_err_t init_timer() {
    if (timer_queue != NULL) {
        return ESP_FAIL;
    }

    timer_queue = xQueueCreate(IBUS_UART_QUEUE_SIZE, sizeof(timer_event_t));
    
    timer_config_t timer_cfg = {
        .divider = IBUS_TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .intr_type = TIMER_INTR_LEVEL,
        .auto_reload = true,
#ifdef TIMER_GROUP_SUPPORTS_XTAL_CLOCK
        .clk_src = TIMER_SRC_CLK_APB
#endif
    };

    timer_init(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, &timer_cfg);
    timer_set_counter_value(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, 0x00000000ULL);
    timer_set_alarm_value(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, IBUS_TIMER_MS_ALARM);
    timer_enable_intr(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);
    timer_isr_register(IBUS_TIMER_GROUP, IBUS_TIMER_IDX, timer_tick_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(IBUS_TIMER_GROUP, IBUS_TIMER_IDX);

    return ESP_OK;
}

static esp_err_t ibus_init_uart(QueueHandle_t uart_queue_handle, uart_port_t uart_num, int rx_pin_num, int tx_pin_num) {
    uart_config_t uart_config = IBUS_UART_DEFAULT_CONFIG();
    
    esp_err_t ret = uart_param_config(uart_num, &uart_config);
    ret |= uart_set_pin(uart_num, tx_pin_num, rx_pin_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ret |= uart_driver_install(uart_num, IBUS_UART_BUFFER_SIZE, IBUS_UART_BUFFER_SIZE, IBUS_UART_QUEUE_SIZE, uart_queue_handle, 0);
    ret |= uart_flush_input(uart_num);

    return ret;
}

static void timer_loop_task(void *arg) {
    timer_event_t evt;
    ibus_ctrl_channel_vals_t channel_vals;
    uint64_t tick_count = 0;
    while(true) {
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);
        tick_count+=evt.timer_counter_value;

        if(ibus_control_update(&channel_vals) == ESP_OK) {
            //
        }

        if (ibus_sensor_update(sensor_handle) == ESP_OK) {
            //
            battery_meter_update();
            uint16_t millivolts = battery_meter_mv();
            extv_sensor.value = millivolts;
        }

        if (tick_count >= 20000) {
            tick_count = 0;
            ESP_LOGI(TAG, "Sensor val: %d, First Channel: %d", extv_sensor.value, channel_vals.channels[0]);
        }
    }
}

esp_err_t ibus_init() {
    ibus_init_uart(control_uart_handle, CONFIG_IBUS_CTRL_UART_NUM, CONFIG_IBUS_CTRL_GPIO, UART_PIN_NO_CHANGE);
    ibus_init_uart(sensor_uart_handle, CONFIG_IBUS_SENSOR_UART_NUM, CONFIG_IBUS_SENSOR_RX_GPIO, CONFIG_IBUS_SENSOR_TX_GPIO);

    sensor_handle = ibus_create_sensor_handle();
    extv_sensor = ibus_create_sensor(IBUS_TYPE_EXTV, 0);
    ibus_push_sensor(sensor_handle, &extv_sensor);

    esp_err_t ret = init_timer();
    if (ret != ESP_OK) {
        return ret;
    }

    battery_meter_init();

    xTaskCreate(timer_loop_task, "ibus_timer_loop_task", 2048, NULL, 5, NULL);

    return ESP_OK;
}