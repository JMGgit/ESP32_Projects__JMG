/*
 * FOTA.c
 *
 *  Created on: 24.05.2017
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "FOTA.h"
#include "uC.h"
#include "IRMP_Appl.h"
#include "Wifi.h"
#include "Clock.h"
#include "ArtNet.h"
#include "Modes.h"

#include "esp_log.h"
#include "sys/socket.h"
#include "esp_ota_ops.h"


#define OTA_WRITE_DATA_BUFSITE 1024
#define HTTP_BUFFER_BUFSIZE 1024

static const char *TAG = "FOTA";
static char otaWriteData[OTA_WRITE_DATA_BUFSITE + 1] = { 0 };
static char httpBuffer[OTA_WRITE_DATA_BUFSITE + 1] = { 0 };
static int binary_file_length = 0;

static uint64_t currentSwVersion;
static uint64_t currentSwVersion_NVS;
static nvs_handle nvsHandle_currentSwVersion;

static uint8_t fotaTrigSwUpdate;
static uint8_t fotaTrigSwUpdate_NVS;
static nvs_handle nvsHandle_fotaTrigSwUpdate;

static uint8_t fotaCheckForNewVersion = FALSE;
static FOTA_State_t fotaState = FOTA_STATE_IDLE;
static FOTA_InternalState_t fotaInternalState = FOTA_INTERNAL_STATE_IDLE;


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

	currentSwVersion = uC__nvsRead_u64("fotaSwVersion", nvsHandle_currentSwVersion, &currentSwVersion_NVS);
	fotaTrigSwUpdate = uC__nvsRead_u8("fotaTrigSwUpd", nvsHandle_fotaTrigSwUpdate, &fotaTrigSwUpdate_NVS);
	uC__nvsUpdate_u8("fotaTrigSwUpd", nvsHandle_fotaTrigSwUpdate, &fotaTrigSwUpdate_NVS, FALSE);

	/* display current SW version and compile time */
	ESP_LOGI(TAG, "Current SW version: %llu\n", FOTA__getCurrentSwVersion());
	ESP_LOGI(TAG, "Compile Date: %s\n", __DATE__);
	ESP_LOGI(TAG, "Compile Time: %s\n\n", __TIME__);
	ESP_LOGI(TAG, "FOTA__init done\n");
}


void FOTA__enableCheck (void)
{
	fotaCheckForNewVersion = TRUE;
}


void FOTA__disableCheck (void)
{
	fotaCheckForNewVersion = FALSE;
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
	uC__nvsUpdate_u8("fotaTrigSwUpd", nvsHandle_fotaTrigSwUpdate, &fotaTrigSwUpdate_NVS, TRUE);
	uC__triggerSwReset();
}


static int FOTA__readUntilDelimiter (char *buffer, char delim, int len)
{
	int i = 0;

	while (buffer[i] != delim && i < len)
	{
		++i;
	}

	return i + 1;
}


static uint8_t FOTA__readPastHttpHeader (char text[], int total_len, esp_ota_handle_t update_handle)
{
	int i = 0, i_read_len = 0;

	while (text[i] != 0 && i < total_len)
	{
		i_read_len = FOTA__readUntilDelimiter(&text[i], '\n', total_len);

		if (i_read_len == 2)
		{
			int i_write_len = total_len - (i + 2);

			memset(otaWriteData, 0, OTA_WRITE_DATA_BUFSITE);
			memcpy(otaWriteData, &(text[i + 2]), i_write_len);

			esp_err_t err = esp_ota_write(update_handle, (const void *) otaWriteData, i_write_len);

			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
				return FALSE;
			}
			else
			{
				ESP_LOGD(TAG, "esp_ota_write header OK");
				binary_file_length += i_write_len;
			}

			return TRUE;
		}

		i += i_read_len;
	}

	return FALSE;
}


static bool FOTA__connectToServerHTTP (const char *serverHostName, const char *serverPort, int *socketId)
{
	int http_connect_flag = -1;
	struct sockaddr_in sock_info;
	uint8_t retVal;

	*socketId = socket(AF_INET, SOCK_STREAM, 0);

	if (*socketId == -1)
	{
		ESP_LOGE(TAG, "Create socket failed!");
		retVal = false;
	}
	else
	{
		memset(&sock_info, 0, sizeof(struct sockaddr_in));
		sock_info.sin_family = AF_INET;
		sock_info.sin_addr.s_addr = inet_addr(serverHostName);
		sock_info.sin_port = htons(atoi(serverPort));

		ESP_LOGI(TAG, "Connect to server IP: %s, server Port:%s", serverHostName, serverPort);

		http_connect_flag = connect(*socketId, (struct sockaddr *) &sock_info, sizeof(sock_info));

		if (http_connect_flag == -1)
		{
			ESP_LOGE(TAG, "Connect to server failed! errno=%d", errno);
			close(*socketId);
			retVal =  false;
		}
		else
		{
			ESP_LOGD(TAG, "Connected to server");
			retVal =  true;
		}
	}

	return retVal;
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


void FOTA__mainFunction(void *param)
{
	esp_err_t err;
	const char *httpGetRequest = "GET %s HTTP/1.0\r\n" "Host: %s:%s\r\n" "User-Agent: ESP32\r\n\r\n";

	static int socketId = -1;

	char *httpRequest = NULL;
	int httpRequestLength = 0;
	int recvBufLength = 0;
	uint8_t waitForData = FALSE;
	uint8_t respBodyStart = FALSE;
	uint8_t taskPause = FALSE;

	uint8_t swVersionReceived = FALSE;
	uint64_t newSwversion = 0;
	char swPath[256];
	uint8_t swPathReceived = FALSE;

	esp_ota_handle_t espOtaHandle = 0;
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

				if ((Wifi__isConnected()) && (fotaCheckForNewVersion || fotaTrigSwUpdate))
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
					ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configuredPartition->address, runningPartition->address);
					ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
					fotaInternalState = FOTA_INTERNAL_STATE_IDLE;
				}
				else
				{
					ESP_LOGD(TAG, "Running partition type %d subtype %d (offset 0x%08x)", runningPartition->type, runningPartition->subtype, runningPartition->address);
					fotaInternalState = FOTA_INTERNAL_STATE_GET_SW_INFO;
				}

				break;
			}

			case FOTA_INTERNAL_STATE_GET_SW_INFO:
			{
				fotaState = FOTA_STATE_CONNECTION_IN_PROGRESS;

				if (FOTA__connectToServerHTTP(FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT, &socketId))
				{
					ESP_LOGI(TAG, "SW Info: Connected to http server");

					httpRequestLength = asprintf(&httpRequest, httpGetRequest, FOTA_SW_INFO_FILE_NAME, FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT);

					if (httpRequestLength < 0)
					{
						ESP_LOGE(TAG, "SW Info: Failed to allocate memory for GET request buffer");
						waitForData = FALSE;
						fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
					}
					else
					{
						if (send(socketId, httpRequest, httpRequestLength, 0) < 0)
						{
							ESP_LOGE(TAG, "SW Info: Send GET request to server failed");
							waitForData = FALSE;
							fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
						}
						else
						{
							/* success */
							ESP_LOGD(TAG, "SW Info: HTTP Request: %s", httpRequest);
							waitForData = TRUE;
						}

						free(httpRequest);
					}

					while (waitForData)
					{
						memset(httpBuffer, 0, HTTP_BUFFER_BUFSIZE);
						recvBufLength = recv(socketId, httpBuffer, HTTP_BUFFER_BUFSIZE, 0);
						ESP_LOGD(TAG, "Buffer left: %d", recvBufLength);

						if (recvBufLength < 0)
						{
							ESP_LOGE(TAG, "SW Info: Error: receive SW version error! errno=%d", errno);
							waitForData = FALSE;
							fotaState = FOTA_STATE_ERROR;
						}
						else if (recvBufLength > 0)
						{
							if (!FOTA__parseKeyValueUint64(httpBuffer, "VERSION=", &newSwversion))
							{
								ESP_LOGI(TAG, "SW Info: New SW version: '%llu'", newSwversion);
								ESP_LOGI(TAG, "SW Info: Current SW version: '%llu'", currentSwVersion);
								swVersionReceived = TRUE;
							}

							if (!FOTA__parseKeyValueString(httpBuffer, "FILE=", swPath, sizeof(swPath) / sizeof(char)))
							{
								ESP_LOGI(TAG, "SW Info: New SW Path '%s'", swPath);
								swPathReceived = TRUE;
							}
						}
						else /* recvBufLength == 0 */
						{
							ESP_LOGI(TAG, "SW Info: all data received");
							waitForData = FALSE;
						}
					}

					close(socketId);
				}
				else
				{
					ESP_LOGE(TAG, "Connect to http server failed!");
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}

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
						ESP_LOGI(TAG, "Same SW version -> nothing to update");
						taskPause = TRUE;
						fotaInternalState = FOTA_INTERNAL_STATE_IDLE;
						fotaState = FOTA_STATE_NO_UPDATE;
					}
				}
				else
				{
					ESP_LOGE(TAG, "Error: not able to parse SW version");
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}

				break;
			}

			case FOTA_INTERNAL_STATE_GET_BINARY:
			{
				fotaState = FOTA_STATE_UPDATE_IN_PROGRESS;

				if (FOTA__connectToServerHTTP(FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT, &socketId))
				{
					ESP_LOGI(TAG, "Binary: Connected to http server");

					httpRequestLength = asprintf(&httpRequest, httpGetRequest, swPath, FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT);

					if (httpRequestLength < 0)
					{
						ESP_LOGE(TAG, "Binary: Failed to allocate memory for GET request buffer");
						fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
					}
					else
					{
						if (send(socketId, httpRequest, httpRequestLength, 0) < 0)
						{
							ESP_LOGE(TAG, "Binary: Send GET request to server failed");
							waitForData = FALSE;
							fotaInternalState = FOTA_INTERNAL_STATE_ERROR;;
						}
						else
						{
							/* success */
							ESP_LOGD(TAG, "Binary: HTTP Request: %s", httpRequest);
							waitForData = TRUE;
						}

						free(httpRequest);
					}

					FOTA__runBeforeSwUpdate();

					update_partition = esp_ota_get_next_update_partition(NULL);
					ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",	update_partition->subtype, update_partition->address);
					assert(update_partition != NULL);

					err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &espOtaHandle);

					if (err != ESP_OK)
					{
						ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
						waitForData = FALSE;
						fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
					}
					else
					{
						waitForData = TRUE;
						binary_file_length = 0;
					}

					while (waitForData)
					{
						memset(httpBuffer, 0, HTTP_BUFFER_BUFSIZE);
						memset(otaWriteData, 0, OTA_WRITE_DATA_BUFSITE);

						recvBufLength = recv(socketId, httpBuffer, HTTP_BUFFER_BUFSIZE, 0);

						if (recvBufLength < 0)
						{
							ESP_LOGE(TAG, "Error: receive SW binary data error! errno=%d", errno);
							waitForData = FALSE;
							fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
						}
						else if ((recvBufLength > 0) && (!respBodyStart))
						{
							memcpy(otaWriteData, httpBuffer, recvBufLength);
							respBodyStart = FOTA__readPastHttpHeader(httpBuffer, recvBufLength,	espOtaHandle);
						}
						else if ((recvBufLength > 0) && (respBodyStart))
						{
							memcpy(otaWriteData, httpBuffer, recvBufLength);
							err = esp_ota_write(espOtaHandle, (const void *) otaWriteData, recvBufLength);

							if (err != ESP_OK)
							{
								ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
								waitForData = FALSE;
								fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
							}

							binary_file_length += recvBufLength;
							ESP_LOGD(TAG, "Have written image length %d", binary_file_length);
							ESP_LOGD(TAG, "Buffer left %d", recvBufLength);
						}
						else if (recvBufLength == 0)
						{
							waitForData = FALSE;
							ESP_LOGI(TAG, "Connection closed, all packets received");
						}
						else
						{
							waitForData = FALSE;
							fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
							ESP_LOGE(TAG, "Unexpected recv result");
						}
					}


					FOTA__runAfterSwUpdate();
					close(socketId);

					ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

					if (esp_ota_end(espOtaHandle) != ESP_OK)
					{
						ESP_LOGE(TAG, "esp_ota_end failed!");
						fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
					}
					else
					{
						/* success */
						err = esp_ota_set_boot_partition(update_partition);

						if (err != ESP_OK)
						{
							ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
							fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
						}
						else
						{
							ESP_LOGI(TAG, "SW updated successfully!");
							FOTA__setCurrentSwVersion(newSwversion);
							fotaInternalState = FOTA_INTERNAL_STATE_SUCCESS;
						}
					}
				}
				else
				{
					ESP_LOGE(TAG, "Binary: Connect to http server failed!");
					fotaInternalState = FOTA_INTERNAL_STATE_ERROR;
				}

				break;
			}

			case FOTA_INTERNAL_STATE_ERROR:
			{
				fotaState = FOTA_STATE_ERROR;
				taskPause = TRUE;
				ESP_LOGE(TAG, "SW update aborted due to fatal error");
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

