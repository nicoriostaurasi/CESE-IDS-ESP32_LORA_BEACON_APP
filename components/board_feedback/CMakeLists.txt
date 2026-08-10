idf_component_register(SRCS "board_feedback.c"
                       INCLUDE_DIRS "include"
                       REQUIRES board_config oled_display status_led esp_timer)
