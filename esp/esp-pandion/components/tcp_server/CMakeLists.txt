idf_component_register(SRCS "tcp_server.c"
                       INCLUDE_DIRS "include"
                       REQUIRES flight_control wifi_ap tcp_server nvs_flash)