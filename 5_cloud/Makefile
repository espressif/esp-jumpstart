#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := 5_cloud

JUMPSTART_BOARD := board_esp8266_devkitc.h
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/../components

include $(IDF_PATH)/make/project.mk
CPPFLAGS += -DJUMPSTART_BOARD=\"$(JUMPSTART_BOARD)\"
