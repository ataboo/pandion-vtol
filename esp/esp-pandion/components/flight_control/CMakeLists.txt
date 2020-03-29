idf_component_register(SRCS "axis_pid.c" "flight_control.c" "axis_curve.c"
                       INCLUDE_DIRS "include"
                       REQUIRES gyro_control ibus_duplex servo_control dshot_esc)
