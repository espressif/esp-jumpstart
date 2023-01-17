#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := 7_mfg

JUMPSTART_BOARD := board_devkits.h
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/../components

include $(IDF_PATH)/make/project.mk
FW_VERSION ?= v1.1
CPPFLAGS += -DFW_VERSION=\"$(FW_VERSION)\" -DJUMPSTART_BOARD=\"$(JUMPSTART_BOARD)\"
