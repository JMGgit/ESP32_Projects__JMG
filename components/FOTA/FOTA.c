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


#define BUFFSIZE 1024
#define TEXT_BUFFSIZE 1024

static const char *TAG = "FOTA";
static char ota_write_data[BUFFSIZE + 1] = { 0 };
static char text[BUFFSIZE + 1] = { 0 };
static int binary_file_length = 0;

static uint64_t currentSwVersion;
static uint64_t currentSwVersion_NVS;
static nvs_handle nvsHandle_currentSwVersion;

static uint8_t fotaTrigSwUpdate;
static uint8_t fotaTrigSwUpdate_NVS;
static nvs_handle nvsHandle_fotaTrigSwUpdate;

static FOTA_State_t fotaState = FOTA_STATE_IDLE;


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
	printf("\nCurrent SW version: %llu\n", FOTA__getCurrentSwVersion());
	printf("Compile Date: %s\n", __DATE__);
	printf("Compile Time: %s\n\n", __TIME__);

	printf("OTA__init done\n");
}


void FOTA__enable (void)
{
	fotaTrigSwUpdate = TRUE;
}


void FOTA__disable (void)
{
	Clock__init();
	ArtNet__init();
	IRMP__enable();
}


static void FOTA__runBeforeSwUpdate (void)
{
	Modes__setMode(MODE__OFF, FALSE);
	Clock__shutdown();
	ArtNet__shutdown();
	IRMP__disable();
}


static void FOTA__runAfterSwUpdate (void)
{
	IRMP__enable();
}


void FOTA__triggerSwUpdate (void)
{
	uC__nvsUpdate_u8("fotaTrigSwUpd", nvsHandle_fotaTrigSwUpdate, &fotaTrigSwUpdate_NVS, TRUE);
	uC__triggerSwReset();
}


uint8_t FOTA__isSwUpdateTriggered (void)
{
	return (fotaTrigSwUpdate && Wifi__isConnected());
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


static bool FOTA__readPastHttpHeader (char text[], int total_len, esp_ota_handle_t update_handle)
{
	int i = 0, i_read_len = 0;

	while (text[i] != 0 && i < total_len)
	{
		i_read_len = FOTA__readUntilDelimiter(&text[i], '\n', total_len);

		if (i_read_len == 2)
		{
			int i_write_len = total_len - (i + 2);

			memset(ota_write_data, 0, BUFFSIZE);
			memcpy(ota_write_data, &(text[i + 2]), i_write_len);

			esp_err_t err = esp_ota_write(update_handle, (const void *) ota_write_data, i_write_len);

			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
				return false;
			}
			else
			{
				ESP_LOGD(TAG, "esp_ota_write header OK");
				binary_file_length += i_write_len;
			}

			return true;
		}

		i += i_read_len;
	}

	return false;
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


static void FOTA__abortOnError (void)
{
	ESP_LOGE(TAG, "Exiting task due to fatal error...");
	fotaState = FOTA_STATE_ERROR;
	FOTA__runAfterSwUpdate();
	vTaskDelete(NULL);
}


static void FOTA__completeSwUpdate (void)
{
	ESP_LOGI(TAG, "SW updated successfully!");
	fotaState = FOTA_STATE_UPDADE_FINISHED;
	fotaTrigSwUpdate = FALSE;
	FOTA__runAfterSwUpdate();
	vTaskDelete(NULL);
}


void FOTA__mainFunction(void *param)
{
	esp_err_t err;
	const char *httpGetRequest = "GET %s HTTP/1.0\r\n" "Host: %s:%s\r\n" "User-Agent: ESP32\r\n\r\n";

	static int socketId = -1;

	char *httpRequest = NULL;
	int httpRequestLength = 0;
	int recvBufLength = 0;
	uint8_t waitForData = TRUE;

	uint8_t swVersionReceived = FALSE;
	uint64_t newSwversion = 0;
	char swPath[256];
	uint8_t swPathReceived = FALSE;

	esp_ota_handle_t espOtaHandle = 0;
	const esp_partition_t *update_partition = NULL;
	const esp_partition_t *configuredPartition = esp_ota_get_boot_partition();
	const esp_partition_t *runningPartition = esp_ota_get_running_partition();

	while (1)
	{
		if (FOTA__isSwUpdateTriggered())
		{
			FOTA__runBeforeSwUpdate();

			if (configuredPartition != runningPartition)
			{
				ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configuredPartition->address, runningPartition->address);
				ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
			}

			ESP_LOGD(TAG, "Running partition type %d subtype %d (offset 0x%08x)", runningPartition->type, runningPartition->subtype, runningPartition->address);

			fotaState = FOTA_STATE_CONNECTION_IN_PROGRESS;

			/***************************/
			/***** request SW infos ****/
			/***************************/

			if (FOTA__connectToServerHTTP(FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT, &socketId))
			{
				ESP_LOGI(TAG, "Connected to http server");
			}
			else
			{
				ESP_LOGE(TAG, "Connect to http server failed!");
				close(socketId);
				FOTA__abortOnError();
			}

			httpRequestLength = asprintf(&httpRequest, httpGetRequest, FOTA_SW_INFO_FILE_NAME, FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT);

			if (httpRequestLength < 0)
			{
				ESP_LOGE(TAG, "SW Info: Failed to allocate memory for GET request buffer");
				close(socketId);
				FOTA__abortOnError();
			}

			if (send(socketId, httpRequest, httpRequestLength, 0) < 0)
			{
				ESP_LOGE(TAG, "SW Info: Send GET request to server failed");
				close(socketId);
				FOTA__abortOnError();
			}
			else
			{
				ESP_LOGD(TAG, "SW Info: HTTP Request: %s", httpRequest);
				ESP_LOGI(TAG, "SW Info: Send GET request to server succeeded");
			}

			free(httpRequest);


			while (waitForData)
			{
				memset(text, 0, TEXT_BUFFSIZE);
				recvBufLength = recv(socketId, text, TEXT_BUFFSIZE, 0);
				ESP_LOGD(TAG, "Buffer left: %d", recvBufLength);

				if (recvBufLength < 0)
				{
					ESP_LOGE(TAG, "SW Info: Error: receive SW version error! errno=%d", errno);
					close(socketId);
					FOTA__abortOnError();
				}
				else if (recvBufLength > 0)
				{
					if (!FOTA__parseKeyValueUint64(text, "VERSION=", &newSwversion))
					{
						ESP_LOGI(TAG, "SW Info: New SW version: '%llu'", newSwversion);
						ESP_LOGI(TAG, "SW Info: Current SW version: '%llu'", currentSwVersion);
						swVersionReceived = TRUE;
					}

					if (!FOTA__parseKeyValueString(text, "FILE=", swPath, sizeof(swPath) / sizeof(char)))
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


			/****************************/
			/***** request SW binary ****/
			/****************************/

			if (FOTA__connectToServerHTTP(FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT, &socketId))
			{
				ESP_LOGI(TAG, "Connected to http server");
			}
			else
			{
				ESP_LOGE(TAG, "Connect to http server failed!");
				close(socketId);
				FOTA__abortOnError();
			}

			if (swVersionReceived && swPathReceived)
			{
				if (newSwversion != currentSwVersion)
				{
					httpRequestLength = asprintf(&httpRequest, httpGetRequest, swPath, FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT);

					if (httpRequestLength < 0)
					{
						ESP_LOGE(TAG, "Binary: Failed to allocate memory for GET request buffer");
						close(socketId);
						FOTA__abortOnError();
					}

					if (send(socketId, httpRequest, httpRequestLength, 0) < 0)
					{
						ESP_LOGE(TAG, "Binary: Send GET request to server failed");
						close(socketId);
						FOTA__abortOnError();
					}
					else
					{
						ESP_LOGD(TAG, "Binary: HTTP Request: %s", httpRequest);
						ESP_LOGI(TAG, "Binary: Send GET request to server succeeded");
					}

					free(httpRequest);

					fotaState = FOTA_STATE_UPDATE_IN_PROGRESS;

					update_partition = esp_ota_get_next_update_partition(NULL);
					ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",	update_partition->subtype, update_partition->address);
					assert(update_partition != NULL);

					err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &espOtaHandle);

					if (err != ESP_OK)
					{
						ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
						close(socketId);
						FOTA__abortOnError();
					}

					ESP_LOGI(TAG, "esp_ota_begin succeeded");

					bool resp_body_start = false, flag = true;

					while (flag)
					{
						memset(text, 0, TEXT_BUFFSIZE);
						memset(ota_write_data, 0, BUFFSIZE);

						recvBufLength = recv(socketId, text, TEXT_BUFFSIZE, 0);

						if (recvBufLength < 0)
						{
							ESP_LOGE(TAG, "Error: receive SW binary data error! errno=%d", errno);
							close(socketId);
							FOTA__abortOnError();
						}
						else if ((recvBufLength > 0) && (!resp_body_start))
						{
							memcpy(ota_write_data, text, recvBufLength);
							resp_body_start = FOTA__readPastHttpHeader(text, recvBufLength,	espOtaHandle);
						}
						else if ((recvBufLength > 0) && (resp_body_start))
						{
							memcpy(ota_write_data, text, recvBufLength);
							err = esp_ota_write(espOtaHandle, (const void *) ota_write_data, recvBufLength);

							if (err != ESP_OK)
							{
								ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
								close(socketId);
								FOTA__abortOnError();
							}

							binary_file_length += recvBufLength;
							ESP_LOGD(TAG, "Have written image length %d", binary_file_length);
							ESP_LOGD(TAG, "Buffer left %d", recvBufLength);
						}
						else if (recvBufLength == 0)
						{
							flag = false;
							ESP_LOGI(TAG, "Connection closed, all packets received");
							close(socketId);
						}
						else
						{
							ESP_LOGE(TAG, "Unexpected recv result");
						}
					}

					ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

					if (esp_ota_end(espOtaHandle) != ESP_OK)
					{
						ESP_LOGE(TAG, "esp_ota_end failed!");
						close(socketId);
						FOTA__abortOnError();
					}

					err = esp_ota_set_boot_partition(update_partition);

					if (err != ESP_OK)
					{
						ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
						close(socketId);
						FOTA__abortOnError();
					}

					FOTA__completeSwUpdate();
				}
			}
			else
			{
				ESP_LOGE(TAG, "Error: not able to parse SW version");
				close(socketId);
				FOTA__abortOnError();
			}
		}
	}
}
