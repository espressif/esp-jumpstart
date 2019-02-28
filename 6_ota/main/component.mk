#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_EMBED_TXTFILES := cloud_cfg/server.cert cloud_cfg/device.cert cloud_cfg/device.key cloud_cfg/deviceid.txt cloud_cfg/endpoint.txt cloud_cfg/github_server.cert

ifndef IDF_CI_BUILD
# Print an error if the certificate/key files are missing
$(COMPONENT_PATH)/cloud_cfg/server.cert $(COMPONENT_PATH)/cloud_cfg/device.cert $(COMPONENT_PATH)/cloud_cfg/device.key $(COMPONENT_PATH)/cloud_cfg/deviceid.txt $(COMPONENT_PATH)/cloud_cfg/endpoint.txt $(COMPONENT_PATH)/cloud_cfg/github_server.cert:
	@echo "Missing file $@. This file identifies the ESP32 to cloud."
	exit 1
else  # IDF_CI_BUILD
# this case is for the internal Continuous Integration build which
# compiles all examples. Add some dummy certs so the example can
# compile (even though it won't work)
$(COMPONENT_PATH)/cloud_cfg/server.cert $(COMPONENT_PATH)/cloud_cfg/device.cert $(COMPONENT_PATH)/cloud_cfg/device.key $(COMPONENT_PATH)/cloud_cfg/deviceid.txt $(COMPONENT_PATH)/cloud_cfg/endpoint.txt $(COMPONENT_PATH)/cloud_cfg/github_server.cert:
	echo "Dummy certificate data for continuous integration" > $@
endif
