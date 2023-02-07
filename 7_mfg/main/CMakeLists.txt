set(COMPONENT_SRCS "app_main.c"
					"app_driver.c"
                    "app_wifi.c"
                    "cloud_aws.c"
                    "aws_custom_utils.c"
                    "ota.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/github_server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/endpoint.txt" TEXT)

component_compile_options("-DFW_VERSION=\"v1.1\"")
