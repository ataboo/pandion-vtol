#include "gyro_control.h"


typedef struct {
    float accel_err_x;
    float accel_err_y;
    float gyro_err_x;
    float gyro_err_y;
    float gyro_err_z;
} gyro_error_t;

typedef struct {
    gyro_error_t* error;
    float roll;
    float pitch;
    float yaw;
    unsigned long last_micros;
    unsigned long current_micros;
} gyro_state_t;

static const char* TAG = "gyro-control";
static uint8_t* io_buffer;
static gyro_state_t* state;
static gyro_values_t* value_buffer;


static void disp_buf(uint8_t *buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

static esp_err_t write_bytes(uint8_t regAddr, size_t size, uint8_t* buffer) {
    if (size == 0 || size > GYRO_READ_BUFFER_LEN) {
        return ESP_ERR_INVALID_SIZE;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_GYRO_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, regAddr, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_write(cmd, buffer, size-1, I2C_MASTER_NACK));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, buffer[size-1], I2C_MASTER_ACK));
    
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    esp_err_t ret = i2c_master_cmd_begin(GYRO_I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t write_byte(uint8_t regAddr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_GYRO_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, regAddr, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    esp_err_t ret = i2c_master_cmd_begin(GYRO_I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t select_register(uint8_t regAddr) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_GYRO_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, regAddr, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    esp_err_t ret = i2c_master_cmd_begin(GYRO_I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t read_bytes(uint8_t regAddr, size_t size, uint8_t* buffer) {
    if (size == 0 || size > GYRO_READ_BUFFER_LEN) {
        return ESP_ERR_INVALID_SIZE;
    }

    esp_err_t ret = select_register(regAddr);
    if (ret != ESP_OK) {
        return ret;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_GYRO_I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));

    if (size > 1) {
        ESP_ERROR_CHECK(i2c_master_read(cmd, buffer, size-1, I2C_MASTER_ACK));
    }
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, buffer + size - 1, I2C_MASTER_NACK));

    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ret = i2c_master_cmd_begin(GYRO_I2C_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t read_byte(uint8_t regAddr, uint8_t *data) {
    return read_bytes(regAddr, 1, data);
}

static uint8_t extract_bit_slice_from_int(uint8_t subject, uint8_t start, uint8_t length) {
    uint8_t mask = ((1 << length) - 1) << (start - length + 1);  // Create a mask of 1's from start to end
    subject &= mask;

    return subject >> (start - length + 1);
}

static uint8_t combine_bit_slice_into_int(uint8_t subject, uint8_t data, uint8_t start, uint8_t length) {
    uint8_t mask = ((1<<length)-1) << (start - length + 1);  // Create a mask of 1's from start to end
    data <<= (start - length + 1);  // Shift data to start position
    data &= mask;  // Keep masked bits in data
    subject &= ~(mask);  // Keep non-masked bits in subject

    return subject | data;  // Combine subject and data
}


static esp_err_t reset_gyro() {
    return write_byte(MPU6050_RA_PWR_MGMT_1, 1<<MPU6050_PWR1_DEVICE_RESET_BIT);
}

static esp_err_t set_gyro_config_values() {
    // Using the XAxis clock, build the full PWR1 register value if we need any other bits set.
    esp_err_t ret = write_byte(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_XAXIS_CLK);

    ret |= write_byte(MPU6050_RA_GYRO_CFG, CONFIG_GYRO_RANGE);
    ret |= write_byte(MPU6050_RA_ACCEL_CFG, CONFIG_ACCEL_RANGE);

    uint8_t config;
    ret |= read_byte(MPU6050_RA_CONFIG, &config);
    config = combine_bit_slice_into_int(config, CONFIG_DLPF_CFG_LEVEL, MPU6050_CFG_DLPF_BIT, MPU6050_CFG_DLPF_LENGTH);
    ret |= write_byte(MPU6050_RA_CONFIG, config);

    return ret;
}

static esp_err_t read_gyro_values(gyro_values_t* values, bool apply_error) {
    esp_err_t ret = read_bytes(MPU6050_RA_ACCEL_XOUT, 6, io_buffer);
    values->norm_accel_x = (float)(io_buffer[0]<<8 | io_buffer[1]);
    values->norm_accel_y = (float)(io_buffer[2]<<8 | io_buffer[3]);
    values->norm_accel_z = (float)(io_buffer[4]<<8 | io_buffer[5]);

    ret |= read_bytes(MPU6050_RA_GYRO_XOUT, 6, io_buffer);
    values->norm_gyro_x = (io_buffer[0]<<8 | io_buffer[1]) / MPU6050_GYRO_LSB;
    values->norm_gyro_y = (io_buffer[2]<<8 | io_buffer[3]) / MPU6050_GYRO_LSB;
    values->norm_gyro_z = (io_buffer[4]<<8 | io_buffer[5]) / MPU6050_GYRO_LSB;

    if (apply_error) {
        values->norm_accel_x -= state->error->accel_err_x;
        values->norm_accel_y -= state->error->accel_err_y;

        values->norm_gyro_x -= state->error->gyro_err_x;
        values->norm_gyro_y -= state->error->gyro_err_y;
        values->norm_gyro_z -= state->error->gyro_err_z;
    }

    values->norm_accel_x /=  MPU6050_ACCEL_LSB;
    values->norm_accel_y /=  MPU6050_ACCEL_LSB;
    values->norm_accel_z /=  MPU6050_ACCEL_LSB;
    values->norm_gyro_x /= MPU6050_GYRO_LSB;
    values->norm_gyro_y /= MPU6050_GYRO_LSB;
    values->norm_gyro_z /= MPU6050_GYRO_LSB;

    state->current_micros = esp_timer_get_time();

    values->delta_micros = state->current_micros - state->last_micros;
    state->last_micros = state->current_micros;

    return ret;
}

static esp_err_t measure_gyro_error(gyro_error_t* gyro_err) {
    gyro_err->gyro_err_x = 0;
    gyro_err->gyro_err_y = 0;
    gyro_err->gyro_err_z = 0;
    gyro_err->accel_err_x = 0;
    gyro_err->accel_err_y = 0;
    
    for(int i=0; i<GYRO_ERROR_SAMPLE_COUNT; i++) {
        esp_err_t ret = read_gyro_values(value_buffer, false);
        if (ret != ESP_OK) {
            return ret;
        }

        gyro_err->accel_err_x = gyro_err->accel_err_x + ((atan((value_buffer->norm_accel_y) / sqrt(pow((value_buffer->norm_accel_x), 2) + pow((value_buffer->norm_accel_y), 2))) * 180 / PI));
        gyro_err->accel_err_y = gyro_err->accel_err_y + ((atan(-1 * (value_buffer->norm_accel_x) / sqrt(pow((value_buffer->norm_accel_y), 2) + pow((value_buffer->norm_accel_z), 2))) * 180 / PI));

        gyro_err->gyro_err_x += value_buffer->norm_gyro_x;
        gyro_err->gyro_err_y += value_buffer->norm_gyro_y;
        gyro_err->gyro_err_z += value_buffer->norm_gyro_z;
    }

    gyro_err->accel_err_x /= GYRO_ERROR_SAMPLE_COUNT;
    gyro_err->accel_err_y /= GYRO_ERROR_SAMPLE_COUNT;
    gyro_err->gyro_err_x /= GYRO_ERROR_SAMPLE_COUNT;
    gyro_err->gyro_err_y /= GYRO_ERROR_SAMPLE_COUNT;
    gyro_err->gyro_err_z /= GYRO_ERROR_SAMPLE_COUNT;

    return ESP_OK;
}

static esp_err_t callibrate_gyro() {
    esp_err_t ret = measure_gyro_error(state->error);
    state->roll = 0;
    state->pitch = 0;
    state->yaw = 0;

    return ret;
}

esp_err_t gyro_control_init() {
    io_buffer = (uint8_t*)malloc(sizeof(uint8_t) * GYRO_READ_BUFFER_LEN);
    value_buffer = (gyro_values_t*)malloc(sizeof(gyro_values_t));
    state = (gyro_state_t*)malloc(sizeof(gyro_state_t));
    state->error = malloc(sizeof(gyro_error_t));

    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (gpio_num_t)CONFIG_GYRO_SDA,
        .scl_io_num = (gpio_num_t)CONFIG_GYRO_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
    };

    config.master.clk_speed = 400000;

    ESP_ERROR_CHECK(i2c_param_config(GYRO_I2C_NUM, &config));
    ESP_ERROR_CHECK(i2c_driver_install(GYRO_I2C_NUM, I2C_MODE_MASTER, 0, 0, 0));

    ESP_ERROR_CHECK_WITHOUT_ABORT(reset_gyro());

    vTaskDelay(500/portTICK_PERIOD_MS);

    ESP_ERROR_CHECK_WITHOUT_ABORT(set_gyro_config_values());

    ESP_ERROR_CHECK_WITHOUT_ABORT(callibrate_gyro());

    return ESP_OK;
}

esp_err_t gyro_check_status() {
    uint8_t status = -1;
    esp_err_t ret = read_byte(MPU6050_RA_WHO_AM_I, &status);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get WHO_AM_I value from MPU6050");
        return ret;
    }

    // Spec says "upper 6 bits of 7-bit I2C address" (0x68=>0x34) but 0 and 7 are reserved at 0 so it should read as 0x68 anyways.
    if (status != CONFIG_GYRO_I2C_ADDRESS) {
        ESP_LOGW(TAG, "Unnexpected WHO_AM_I response from MPU6050: %02x", status);
        return ESP_ERR_INVALID_RESPONSE;
    }

    uint8_t power_val = -1;
    ESP_ERROR_CHECK_WITHOUT_ABORT(read_byte(MPU6050_RA_PWR_MGMT_1, &power_val));

    return ESP_OK;
}

static esp_err_t track_value_position() {
    return ESP_OK;
}

esp_err_t gyro_control_read(gyro_values_t* values) {
    esp_err_t ret = read_gyro_values(values, true);
    if (ret != ESP_OK) {
        return ret;
    }

    // calculate axes

    return ESP_OK;
}

esp_err_t gyro_control_terminate() {
    free(io_buffer);
    free(value_buffer);

    return ESP_OK;
}