#pragma once

#include "esp_log.h"
#include "driver/i2c.h"
#include <math.h>


#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define GYRO_I2C_NUM I2C_NUM_0

#define GYRO_READ_BUFFER_LEN            16
#define GYRO_ERROR_SAMPLE_COUNT         500

#define MPU6050_RA_WHO_AM_I             0x75
#define MPU6050_RA_PWR_MGMT_1           0x6B
#define MPU6050_RA_GYRO_CFG             0x1B
#define MPU6050_RA_ACCEL_CFG            0x1C
#define MPU6050_RA_GYRO_XOUT            0x43
#define MPU6050_RA_ACCEL_XOUT           0x3B
#define MPU6050_RA_CONFIG               0x1A

#define MPU6050_PWR1_DEVICE_RESET_BIT   7
#define MPU6050_PWR1_CLKSEL_BIT         2
#define MPU6050_PWR1_CLKSEL_LENGTH      3

#define MPU6050_CFG_DLPF_BIT            2
#define MPU6050_CFG_DLPF_LENGTH         3

#define MPU6050_GYRO_RANGE_BIT          4
#define MPU6050_GYRO_RANGE_LENGTH       2

#define MPU6050_ACCEL_RANGE_BIT         4
#define MPU6050_ACCEL_RANGE_LENGTH      2

#define MPU6050_PWR1_XAXIS_CLK          0x01

#define GYRO_LOW_RANGE                  0
#define GYRO_MED_RANGE                  1
#define GYRO_HIGH_RANGE                 2
#define GYRO_XHIGH_RANGE                3

#if CONFIG_GYRO_RANGE == GYRO_LOW_RANGE
    #define MPU6050_GYRO_LSB      131
#elif CONFIG_GYRO_RANGE == GYRO_MED_RANGE
    #define MPU6050_GYRO_LSB      65.5
#elif CONFIG_GYRO_RANGE == GYRO_HIGH_RANGE
    #define MPU6050_GYRO_LSB      32.8
#elif CONFIG_GYRO_RANGE == GYRO_XHIGH_RANGE
    #define MPU6050_GYRO_LSB      16.4
#endif

#if CONFIG_ACCEL_RANGE == GYRO_LOW_RANGE
    #define MPU6050_ACCEL_LSB     16384
#elif CONFIG_ACCEL_RANGE == GYRO_MED_RANGE
    #define MPU6050_ACCEL_LSB     8192
#elif CONFIG_ACCEL_RANGE == GYRO_HIGH_RANGE
    #define MPU6050_ACCEL_LSB     4096
#elif CONFIG_GYRO_RANGE == GYRO_XHIGH_RANGE
    #define MPU6050_ACCEL_LSB     2048
#endif

typedef struct {
    float norm_accel_x;
    float norm_accel_y;
    float norm_accel_z;
    float accel_x_component;
    float accel_y_component;

    float roll_rads;
    float pitch_rads;
    float yaw_rads;
    float norm_gyro_x;
    float norm_gyro_y;
    float norm_gyro_z;
    uint32_t delta_micros;
} gyro_values_t;

esp_err_t gyro_control_init();

esp_err_t gyro_control_read(gyro_values_t* values);

esp_err_t gyro_control_terminate();

esp_err_t gyro_check_status();

