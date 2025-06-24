idf_component_register(SRCS "app_main.c"
                                "app_driver.c"
                                "app_wifi.c"
                                "cloud_aws.c"
                                "aws_custom_utils.c"
                       INCLUDE_DIRS ""
                       REQUIRES driver nvs_flash esp_wifi esp-aws-iot)

target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.key" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/deviceid.txt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/endpoint.txt" TEXT)
