set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_PRIV_INCLUDEDIRS src)
set(COMPONENT_SRCS  "src/conn_mgr_prov.c"
					"src/conn_mgr_prov_handlers.c"
					"src/conn_mgr_prov_mode_ble.c"
					"src/conn_mgr_prov_mode_softap.c")

set(COMPONENT_REQUIRES protocomm)
set(COMPONENT_PRIV_REQUIRES wifi_provisioning nvs_flash)

register_component()
