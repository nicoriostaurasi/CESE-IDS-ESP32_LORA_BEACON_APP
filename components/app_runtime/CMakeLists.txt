idf_component_register(SRCS "app_runtime.c"
                       INCLUDE_DIRS "include"
                       REQUIRES beacon_metrics beacon_protocol board_config board_feedback esp_driver_spi esp_timer sx1262_radio)
