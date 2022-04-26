set(COMPONENT_SRCS "app_main.c"
					"app_driver.c"
                    "app_wifi.c"
					"cloud_aws.c"
                    "aws_custom_utils.c")
set(COMPONENT_ADD_INCLUDEDIRS "")
set(JUMPSTART_BOARD "board_esp32_devkitc.h")

register_component()

target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.key" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/deviceid.txt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/endpoint.txt" TEXT)

component_compile_options("-DJUMPSTART_BOARD=\"${JUMPSTART_BOARD}\"")
