#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := 3_wifi_connection

JUMPSTART_BOARD := board_esp32_devkitc.h
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/../components

include $(IDF_PATH)/make/project.mk
CPPFLAGS += -DJUMPSTART_BOARD=\"$(JUMPSTART_BOARD)\"
