#include "stdio.h"
#include "app_priv.h"
#include "aws_custom_utils.h"
#include "string.h"

#define OBJECT_NAME_STRING "\"%s\":{"

static inline IoT_Error_t check_snprintf_ret_val(int32_t snPrintfReturn, size_t maxSizeOfJsonDocument) {
	if(snPrintfReturn < 0) {
		return SHADOW_JSON_ERROR;
	} else if((size_t) snPrintfReturn >= maxSizeOfJsonDocument) {
		return SHADOW_JSON_BUFFER_TRUNCATED;
	}
	return SUCCESS;
}

static IoT_Error_t convert_data_to_string(char *pStringBuffer, size_t maxSizeofStringBuffer, JsonPrimitiveType type,
									   void *pData) {
	int32_t snPrintfReturn = 0;
	IoT_Error_t ret_val = SUCCESS;

	if(maxSizeofStringBuffer == 0) {
		return SHADOW_JSON_ERROR;
	}

	if(type == SHADOW_JSON_INT32) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%i,", *(int32_t *) (pData));
	} else if(type == SHADOW_JSON_INT16) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%hi,", *(int16_t *) (pData));
	} else if(type == SHADOW_JSON_INT8) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%hhi,", *(int8_t *) (pData));
	} else if(type == SHADOW_JSON_UINT32) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%u,", *(uint32_t *) (pData));
	} else if(type == SHADOW_JSON_UINT16) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%hu,", *(uint16_t *) (pData));
	} else if(type == SHADOW_JSON_UINT8) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%hhu,", *(uint8_t *) (pData));
	} else if(type == SHADOW_JSON_DOUBLE) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%f,", *(double *) (pData));
	} else if(type == SHADOW_JSON_FLOAT) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%f,", *(float *) (pData));
	} else if(type == SHADOW_JSON_BOOL) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%s,", *(bool *) (pData) ? "true" : "false");
	} else if(type == SHADOW_JSON_STRING) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "\"%s\",", (char *) (pData));
	} else if(type == SHADOW_JSON_OBJECT) {
		snPrintfReturn = snprintf(pStringBuffer, maxSizeofStringBuffer, "%s,", (char *) (pData));
	}

	ret_val = check_snprintf_ret_val(snPrintfReturn, maxSizeofStringBuffer);

	return ret_val;
}

static IoT_Error_t generate_json_object(char *object_name, char *pJsonDocument, size_t maxSizeOfJsonDocument, uint8_t count, jsonStruct_t **handler) {
	IoT_Error_t ret_val = SUCCESS;
	size_t tempSize = 0;
	int8_t i;
	jsonStruct_t *pTemporary = NULL;
	size_t remSizeOfJsonBuffer = maxSizeOfJsonDocument;
	int32_t snPrintfReturn = 0;

	if(pJsonDocument == NULL) {
		return NULL_VALUE_ERROR;
	}

	tempSize = maxSizeOfJsonDocument - strlen(pJsonDocument);
	if(tempSize <= 1) {
		return SHADOW_JSON_ERROR;
	}
	remSizeOfJsonBuffer = tempSize;

	snPrintfReturn = snprintf(pJsonDocument + strlen(pJsonDocument), remSizeOfJsonBuffer, OBJECT_NAME_STRING, object_name);
	ret_val = check_snprintf_ret_val(snPrintfReturn, maxSizeOfJsonDocument);
	if (ret_val != SUCCESS) {
		return ret_val;
	}
	for(i = 0; i < count; i++) {
		tempSize = maxSizeOfJsonDocument - strlen(pJsonDocument);
		if(tempSize <= 1) {
			return SHADOW_JSON_ERROR;
		}
		remSizeOfJsonBuffer = tempSize;
		pTemporary = (jsonStruct_t *)handler[i];
		if(pTemporary != NULL) {
			snPrintfReturn = snprintf(pJsonDocument + strlen(pJsonDocument), remSizeOfJsonBuffer, "\"%s\":",
									  pTemporary->pKey);
			if (snPrintfReturn < 0) {
				return NULL_VALUE_ERROR;
			}
			if(ret_val != SUCCESS) {
				return ret_val;
			}
			if(pTemporary->pKey != NULL && pTemporary->pData != NULL) {				
                ret_val = convert_data_to_string(pJsonDocument + strlen(pJsonDocument), remSizeOfJsonBuffer,
											  pTemporary->type, pTemporary->pData);
			} else {
				return NULL_VALUE_ERROR;
			}
			if(ret_val != SUCCESS) {
				return ret_val;
			}
		} else {
			return NULL_VALUE_ERROR;
		}
	}

	snPrintfReturn = snprintf(pJsonDocument + strlen(pJsonDocument) - 1, remSizeOfJsonBuffer, "},");
	ret_val = check_snprintf_ret_val(snPrintfReturn, maxSizeOfJsonDocument);
	if (ret_val != SUCCESS) {
		return ret_val;
	}	

	return ret_val;
}

IoT_Error_t custom_aws_iot_shadow_add_desired(char *pJsonDocument,
											  size_t maxSizeOfJsonDocument,
											  uint8_t count,
											  jsonStruct_t **handler)
{
	return generate_json_object("desired", pJsonDocument, maxSizeOfJsonDocument, count, handler);
}

IoT_Error_t custom_aws_iot_shadow_add_reported(char *pJsonDocument,
					     size_t maxSizeOfJsonDocument,
						 uint8_t count, 
						 jsonStruct_t **handler)
{
	return generate_json_object("reported", pJsonDocument, maxSizeOfJsonDocument, count, handler);
}
