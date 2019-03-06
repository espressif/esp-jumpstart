set(COMPONENT_SRCS "app_main.c"
					"app_driver.c"
	)
set(COMPONENT_ADD_INCLUDEDIRS "")
set(JUMPSTART_BOARD "board_esp32_devkitc.h")

register_component()

component_compile_options("-DJUMPSTART_BOARD=\"${JUMPSTART_BOARD}\"")
