#pragma once

#include "stdint.h"
#include "aws_iot_error.h"
#include "aws_iot_shadow_json_data.h"

IoT_Error_t custom_aws_iot_shadow_add_desired(char *pJsonDocument,
                        size_t maxSizeOfJsonDocument,
                        uint8_t count,
                        jsonStruct_t **handler);
IoT_Error_t custom_aws_iot_shadow_add_reported(char *pJsonDocument,
                        size_t maxSizeOfJsonDocument,
                        uint8_t count,
                        jsonStruct_t **handler);
