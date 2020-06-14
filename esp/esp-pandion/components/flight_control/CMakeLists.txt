idf_component_register(SRCS "flight_control.c" "timer.c"
                       INCLUDE_DIRS "include"
                       REQUIRES gyro_control ibus_uart servo_control dshot_esc battery_meter esp_adc_cal axis_pid axis_curve neutral_axis_stabilizer flight_control_common positive_axis_stabilizer)
