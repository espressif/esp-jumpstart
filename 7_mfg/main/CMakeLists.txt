idf_component_register(SRCS "app_main.c"
                                "app_driver.c"
                                "app_wifi.c"
                                "app_cloud.c"
                                "app_ota.c"
                       INCLUDE_DIRS ""
                       REQUIRES driver nvs_flash esp_wifi mqtt esp_https_ota)

target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.key" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/deviceid.txt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/endpoint.txt" TEXT)

component_compile_options("-DFW_VERSION=\"v1.1\"")
