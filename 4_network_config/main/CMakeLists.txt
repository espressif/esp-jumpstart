idf_component_register(SRCS "app_main.c"
                                "app_driver.c"
                                "app_wifi.c"
                       INCLUDE_DIRS ""
                       REQUIRES driver nvs_flash esp_wifi)
