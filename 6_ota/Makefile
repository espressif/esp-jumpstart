#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := 6_ota

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/../components

include $(IDF_PATH)/make/project.mk
VERSION ?= v1.1
CPPFLAGS += -DVERSION=\"$(VERSION)\"
