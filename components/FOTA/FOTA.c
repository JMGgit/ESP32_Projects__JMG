/*
 * FOTA.c
 *
 *  Created on: 24.05.2017
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "FOTA"

#include "FOTA.h"
#include "uC.h"
#include "IRMP_Appl.h"
#include "Wifi.h"
#include "Clock.h"
#include "ArtNet.h"
#include "Modes.h"

#include "esp_log.h"
#include "sys/socket.h"
#include "netdb.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"


#define OTA_WRITE_DATA_BUFSIZE 1024
#define HTTP_BUFFER_BUFSIZE 1024

#define FOTA_HTTP_REQUEST_SWINFO	1
#define FOTA_HTTP_REQUEST_BINARY	2

static esp_ota_handle_t espOtaHandle = 0;

static int binary_file_length = 0;

static uint64_t currentSwVersion;
static uint64_t currentSwVersion_NVS;
static nvs_handle nvsHandle_currentSwVersion;

static uint8_t fotaTrigSwUpdate;
static uint8_t fotaTrigSwUpdate_NVS;
static nvs_handle nvsHandle_fotaTrigSwUpdate;

static uint8_t fotaCyclCheck;
static uint8_t fotaCyclCheck_NVS;
static nvs_handle nvsHandle_fotaCyclCheck;
static uint8_t fotaCyclCheckTemp;
static FOTA_State_t fotaState = FOTA_STATE_IDLE;
static FOTA_InternalState_t fotaInternalState = FOTA_INTERNAL_STATE_IDLE;
static uint32_t fotaErrorsCount = 0;

uint8_t swVersionReceived = FALSE;
uint64_t newSwversion = 0;
char swPath[256];
uint8_t swPathReceived = FALSE;


FOTA_State_t FOTA__getCurrentState (void)
{
	return fotaState;
}


uint64_t FOTA__getCurrentSwVersion (void)
{
	return currentSwVersion;
}


static void FOTA__setCurrentSwVersion (uint64_t newSwVersion)
{
	currentSwVersion = newSwVersion;
	uC__nvsUpdate_u64("fotaSwVersion", nvsHandle_currentSwVersion, &currentSwVersion_NVS, newSwVersion);
}


void FOTA__init (void)
{
	uC__nvsInitStorage("fotaSwVersion", &nvsHandle_currentSwVersion);
	uC__nvsInitStorage("fotaTrigSwUpd", &nvsHandle_fotaTrigSwUpdate);
	uC__nvsInitStorage("fotaCyclCheck", &nvsHandle_fotaCyclCheck);

	currentSwVersion = uC__nvsRead_u64("fotaSwVersion", nvsHandle_currentSwVersion, &currentSwVersion_NVS);
	fotaTrigSwUpdate = uC__nvsRead_u8("fotaTrigSwUpd", nvsHandle_fotaTrigSwUpdate, &fotaTrigSwUpdate_NVS);
	fotaCyclCheck = uC__nvsRead_u8("fotaCyclCheck", nvsHandle_fotaCyclCheck, &fotaCyclCheck_NVS);

	if (fotaCyclCheck == 255)
	{
		fotaCyclCheck = TRUE;
		uC__nvsUpdate_u8("fotaCyclCheck", nvsHandle_fotaCyclCheck, &fotaCyclCheck_NVS, fotaCyclCheck);
	}

	uC__nvsUpdate_u8("fotaTrigSwUpd", nvsHandle_fotaTrigSwUpdate, &fotaTrigSwUpdate_NVS, FALSE);

	/* display current SW version and compile time */
	ESP_LOGI(LOG_TAG, "Current SW version: %llu", FOTA__getCurrentSwVersion());
	ESP_LOGI(LOG_TAG, "Compile Date: %s", __DATE__);
	ESP_LOGI(LOG_TAG, "Compile Time: %s", __TIME__);
	ESP_LOGI(LOG_TAG, "FOTA cyclic check state: %d", fotaCyclCheck);
	ESP_LOGI(LOG_TAG, "FOTA__init done");
}


void FOTA__toggleCyclicCheck (void)
{
	if (fotaCyclCheck)
	{
		fotaCyclCheck = FALSE;
	}
	else
	{
		fotaCyclCheck = TRUE;
	}

	uC__nvsUpdate_u8("fotaCyclCheck", nvsHandle_fotaCyclCheck, &fotaCyclCheck_NVS, fotaCyclCheck);
}


void FOTA__enableCyclicCheck (void)
{
	fotaCyclCheck = TRUE;
	uC__nvsUpdate_u8("fotaCyclCheck", nvsHandle_fotaCyclCheck, &fotaCyclCheck_NVS, fotaCyclCheck);
}


void FOTA__enableCyclicCheckTemp (void)
{
	fotaCyclCheckTemp = TRUE;
}


void FOTA__disableCylicCheck (void)
{
	fotaCyclCheck = FALSE;
	uC__nvsUpdate_u8("fotaCyclCheck", nvsHandle_fotaCyclCheck, &fotaCyclCheck_NVS, fotaCyclCheck);
}


void FOTA__disableCyclicCheckTemp (void)
{
	fotaCyclCheckTemp = FALSE;
}


static void FOTA__runBeforeSwUpdate (void)
{
	Modes__setMode(MODE__FOTA, FALSE);
	Clock__shutdown();
	ArtNet__shutdown();
	IRMP__disable();
}


static void FOTA__runAfterSwUpdate (void)
{
	Clock__init();
	ArtNet__init();
	IRMP__enable();

}


void FOTA__triggerSwUpdate (void)
{
	fotaTrigSwUpdate = TRUE;
	uC__nvsUpdate_u8("fotaTrigSwUpd", nvsHandle_fotaTrigSwUpdate, &fotaTrigSwUpdate_NVS, TRUE);
	uC__triggerSwReset();
}



static int FOTA__parseKeyValueUint64 (const char *buffer, const char *key, uint64_t *value)
{
	const char *locKey = strstr(buffer, key);
	uint8_t retVal;

	if (!locKey)
	{
		retVal = -1;
	}
	else
	{
		*value = atoll(&locKey[strlen(key)]);
		retVal = 0;
	}

	return retVal;
}


static int FOTA__parseKeyValueString (const char *buffer, const char *key, char *str, int strLen)
{
	const char *locKey = strstr(buffer, key);
	uint8_t retVal;

	if (!locKey)
	{
		retVal = -1;
	}
	else
	{
		const char *src = &locKey[strlen(key)];

		for (int i = 0; i < strLen - 1; i++)
		{
			if (*src == 0x00 || *src == '\r' || *src == '\n')
			{
				break;
			}

			*str++ = *src++;
		}

		*str++ = 0x00;
		retVal = 0;
	}

	return retVal;
}


static esp_err_t FOTA__httpEventHandler (uint8_t requestType, esp_http_client_event_t *evt)
{
	switch (evt->event_id)
	{
		case HTTP_EVENT_ERROR:
		{
			ESP_LOGD(LOG_TAG, "HTTP_EVENT_ERROR");
			break;
		}

		case HTTP_EVENT_ON_CONNECTED:
		{
			ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_CONNECTED");
			break;
		}

		case HTTP_EVENT_HEADER_SENT:
		{
			ESP_LOGD(LOG_TAG, "HTTP_EVENT_HEADER_SENT");
			break;
		}

		case HTTP_EVENT_ON_HEADER:
		{
			ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
			break;
		}

		case HTTP_EVENT_ON_DATA:
		{
			if (!esp_http_client_is_chunked_response(evt->client))
			{
				switch (requestType)
				{
					case FOTA_HTTP_REQUEST_SWINFO:
					{
						if (!FOTA__parseKeyValueUint64(evt->data, "VERSION=", &newSwversion))
						{
							ESP_LOGI(LOG_TAG, "SW Info: New SW version: '%llu'", newSwversion);
							ESP_LOGI(LOG_TAG, "SW Info: Current SW version: '%llu'", currentSwVersion);
							swVersionReceived = TRUE;
						}

						if (!FOTA__parseKeyValueString(evt->data, "FILE=", swPath, sizeof(swPath) / sizeof(char)))
						{
							ESP_LOGI(LOG_TAG, "SW Info: New SW Path '%s'", swPath);
							swPathReceived = TRUE;
						}

						break;
					}

					case FOTA_HTTP_REQUEST_BINARY:
					{
						ESP_LOGD(LOG_TAG, "Binary data length (chunk): %d", evt->data_len);

						esp_err_t err = esp_ota_write(espOtaHandle, evt->data, evt->data_len);

						if (err != ESP_OK)
						{
							ESP_LOGE(LOG_TAG, "Error: esp_ota_write failed! err=0x%x", err);
							fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
						}

						binary_file_length += evt->data_len;
						ESP_LOGI(LOG_TAG, "Have written image length %d", binary_file_length);

						break;
					}

					default:
					{
						ESP_LOGE(LOG_TAG, "FOTA__httpEventHandler: unknown data received");
					}
				}
			}
			else
			{
				ESP_LOGD(LOG_TAG, "Binary data length (no chunk): %d", evt->data_len);
			}

			break;
		}

		case HTTP_EVENT_ON_FINISH:
		{
			ESP_LOGD(LOG_TAG, "HTTP_EVENT_ON_FINISH");
			break;
		}

		case HTTP_EVENT_DISCONNECTED:
		{
			ESP_LOGD(LOG_TAG, "HTTP_EVENT_DISCONNECTED");
			break;
		}
	}

	return ESP_OK;
}


static esp_err_t FOTA__httpEventHandler_SwInfo(esp_http_client_event_t *evt)
{
	return FOTA__httpEventHandler(FOTA_HTTP_REQUEST_SWINFO, evt);
}


static esp_err_t FOTA__httpEventHandler_Binary(esp_http_client_event_t *evt)
{
	return FOTA__httpEventHandler(FOTA_HTTP_REQUEST_BINARY, evt);
}


void FOTA__mainFunction(void *param)
{
	esp_err_t err;
	uint8_t taskPause = FALSE;

	const esp_partition_t *update_partition = NULL;
	const esp_partition_t *configuredPartition;
	const esp_partition_t *runningPartition;

	while (1)
	{
		while (!taskPause)
		{
			switch (fotaInternalState)
			{

			case FOTA_INTERNAL_STATE_IDLE:
			{
				/* don't set fotaState to FOTA_STATE_IDLE to display last result with led matrix */

				if ((Wifi__isConnected()) && ((fotaCyclCheck && fotaCyclCheckTemp) || fotaTrigSwUpdate))
				{
					fotaTrigSwUpdate = FALSE;
					taskPause = FALSE;
					fotaInternalState = FOTA_INTERNAL_STATE_CHECK_PARTITION;
				}

				break;
			}

			case FOTA_INTERNAL_STATE_CHECK_PARTITION:
			{
				fotaState = FOTA_STATE_IDLE;

				configuredPartition =  esp_ota_get_boot_partition();
				runningPartition = esp_ota_get_running_partition();

				if (configuredPartition != runningPartition)
				{
					ESP_LOGW(LOG_TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configuredPartition->address, runningPartition->address);
					ESP_LOGW(LOG_TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
					fotaInternalState = FOTA_INTERNAL_STATE_IDLE;
				}
				else
				{
					ESP_LOGD(LOG_TAG, "Running partition type %d subtype %d (offset 0x%08x)", runningPartition->type, runningPartition->subtype, runningPartition->address);
					fotaInternalState = FOTA_INTERNAL_STATE_GET_SW_INFO;
				}

				break;
			}

			case FOTA_INTERNAL_STATE_GET_SW_INFO:
			{
				fotaState = FOTA_STATE_CONNECTION_IN_PROGRESS;

				esp_http_client_config_t config = {
						.url = "http://ds216j-jmg.synology.me/esp32/SW_Info.txt",
						.event_handler = FOTA__httpEventHandler_SwInfo,
				};

				esp_http_client_handle_t client = esp_http_client_init(&config);
				esp_err_t err = esp_http_client_perform(client);

				if (err == ESP_OK)
				{
					ESP_LOGI(LOG_TAG, "HTTP chunk encoding Status = %d, content_length = %d",
							esp_http_client_get_status_code(client),
							esp_http_client_get_content_length(client));

					if (esp_http_client_get_status_code(client) != 200)
					{
						ESP_LOGI(LOG_TAG, "SwInfo: Status code not 200");
						fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
					}
				}
				else
				{
					ESP_LOGE(LOG_TAG, "Error perform http request %s", esp_err_to_name(err));
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}

				esp_http_client_cleanup(client);

				fotaInternalState = FOTA_INTERNAL_STATE_CHECK_SW_INFO;

				break;
			}

			case FOTA_INTERNAL_STATE_CHECK_SW_INFO:
			{
				fotaState = FOTA_STATE_IDLE;

				if (swVersionReceived && swPathReceived)
				{
					if (newSwversion != currentSwVersion)
					{
						fotaInternalState = FOTA_INTERNAL_STATE_GET_BINARY;
					}
					else
					{
						ESP_LOGI(LOG_TAG, "Same SW version -> nothing to update");
						taskPause = TRUE;
						fotaInternalState = FOTA_INTERNAL_STATE_IDLE;
						fotaState = FOTA_STATE_NO_UPDATE;
					}
				}
				else
				{
					ESP_LOGE(LOG_TAG, "Error: not able to parse SW version");
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}

				break;
			}

			case FOTA_INTERNAL_STATE_GET_BINARY:
			{
				fotaState = FOTA_STATE_UPDATE_IN_PROGRESS;

				FOTA__runBeforeSwUpdate();

				update_partition = esp_ota_get_next_update_partition(NULL);
				ESP_LOGI(LOG_TAG, "Writing to partition subtype %d at offset 0x%x",	update_partition->subtype, update_partition->address);
				assert(update_partition != NULL);

				err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &espOtaHandle);

				if (err != ESP_OK)
				{
					ESP_LOGE(LOG_TAG, "esp_ota_begin failed, error=%d", err);
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}
				else
				{
					binary_file_length = 0;
				}

				esp_http_client_config_t config = {
						.url = "http://ds216j-jmg.synology.me/esp32/ESP32.bin",
						.event_handler = FOTA__httpEventHandler_Binary,
						.buffer_size = OTA_WRITE_DATA_BUFSIZE,
				};

				esp_http_client_handle_t client = esp_http_client_init(&config);
				esp_err_t err = esp_http_client_perform(client);

				if (err == ESP_OK)
				{
					ESP_LOGI(LOG_TAG, "HTTP chunk encoding Status = %d, content_length = %d",
							esp_http_client_get_status_code(client),
							esp_http_client_get_content_length(client));

					if (esp_http_client_get_status_code(client) != 200)
					{
						ESP_LOGI(LOG_TAG, "Binary: Status code not 200");
						fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
					}
				}
				else
				{
					ESP_LOGE(LOG_TAG, "Binary: Connect to http server failed! %s", esp_err_to_name(err));
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}

				esp_http_client_cleanup(client);


				FOTA__runAfterSwUpdate();

				ESP_LOGI(LOG_TAG, "Total Write binary data length : %d", binary_file_length);

				if (esp_ota_end(espOtaHandle) != ESP_OK)
				{
					ESP_LOGE(LOG_TAG, "esp_ota_end failed!");
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}
				else
				{
					/* success */
					err = esp_ota_set_boot_partition(update_partition);

					if (err != ESP_OK)
					{
						ESP_LOGE(LOG_TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
						fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
					}
					else
					{
						ESP_LOGI(LOG_TAG, "SW updated successfully!");
						FOTA__setCurrentSwVersion(newSwversion);
						fotaInternalState = FOTA_INTERNAL_STATE_SUCCESS;
					}
				}

				break;
			}

			case FOTA_INTERNAL_STATE_ERROR:
			{
				fotaState = FOTA_STATE_ERROR;
				fotaErrorsCount++;
				taskPause = TRUE;
				ESP_LOGE(LOG_TAG, "SW update aborted due to fatal error. Error count: %d", fotaErrorsCount);
				fotaInternalState = FOTA_INTERNAL_STATE_IDLE;
				break;
			}

			case FOTA_INTERNAL_STATE_SUCCESS:
			{
				fotaState = FOTA_STATE_UPDADE_FINISHED;
				taskPause = TRUE;
				break;
			}

			default:
			{
				/* do nothing */
				break;
			}

			} /* switch case */
		}

		taskPause = FALSE;
		vTaskDelay((FOTA_CHECK_UPDATE_INTERVAL_SECS * 1000) / portTICK_PERIOD_MS);

	} /* while */
}

