set(COMPONENT_SRCS "app_main.c"
					"app_driver.c"
                    "cloud.c"
                    "ota.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

target_add_binary_data(${COMPONENT_NAME} "certs/aws-root-ca.pem" TEXT)
target_add_binary_data(${COMPONENT_NAME} "certs/certificate.pem.crt" TEXT)
target_add_binary_data(${COMPONENT_NAME} "certs/private.pem.key" TEXT)
target_add_binary_data(${COMPONENT_NAME} "certs/github_ca_cert.pem" TEXT)
