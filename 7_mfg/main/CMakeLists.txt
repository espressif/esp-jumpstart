set(COMPONENT_SRCS "app_main.c"
					"app_driver.c"
                    "cloud_aws.c"
                    "aws_custom_utils.c"
                    "ota.c")
set(COMPONENT_ADD_INCLUDEDIRS "")
set(JUMPSTART_BOARD "board_esp32_devkitc.h")

register_component()

target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/github_server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/endpoint.txt" TEXT)

component_compile_options("-DFW_VERSION=\"v1.1\"")
component_compile_options("-DJUMPSTART_BOARD=\"${JUMPSTART_BOARD}\"")
