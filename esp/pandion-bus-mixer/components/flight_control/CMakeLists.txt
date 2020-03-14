idf_component_register(SRCS "axis_pid.c" "control_mixer.c" "flight_control.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES gyro_control ibus_duplex)
