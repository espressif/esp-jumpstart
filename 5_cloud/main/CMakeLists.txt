set(COMPONENT_SRCS "app_main.c"
					"app_driver.c"
					"cloud.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/server.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.cert" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/device.key" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/deviceid.txt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "cloud_cfg/endpoint.txt" TEXT)
