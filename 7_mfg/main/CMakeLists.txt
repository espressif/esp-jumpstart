set(COMPONENT_SRCS "app_main.c"
					"app_driver.c"
                    "cloud.c"
                    "ota.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/github_ca_cert.pem" TEXT)
component_compile_options("-DVERSION=\"v1.1\"")
