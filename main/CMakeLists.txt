idf_component_register(SRCS "app_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES app_runtime board_config esp_app_format spi_flash)
