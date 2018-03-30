/*
 * Ota.c
 *
 *  Created on: 24.05.2017
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "OTA.h"
#include "freertos/task.h"
#include "IRMP_Appl.h"
#include "Wifi.h"

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"

#include "nvs.h"
#include "nvs_flash.h"


#define BUFFSIZE 2048
#define TEXT_BUFFSIZE 2048

static const char *TAG = "ota";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
/*an packet receive buffer*/
static char text[BUFFSIZE + 1] = { 0 };
/* an image total length*/
static int binary_file_length = 0;
/*socket id*/
static int socket_id = -1;


static iap_https_config_t ota_config;

static uint64_t otaSwVersion_NVS;
static nvs_handle nvsHandle_otaSwVersion;

static uint8_t otaTrigSwUpdate_NVS;
static nvs_handle nvsHandle_otaTrigSwUpdate;


static OTA_State_t fotaState = FOTA_STATE_IDLE;


OTA_State_t FOTA__getCurrentState (void)
{
	return fotaState;
}


uint64_t FOTA__getCurrentSwVersion (void)
{
	return ota_config.current_software_version;
}


void FOTA__setCurrentSwVersion (uint64_t newSwVersion)
{
	ota_config.current_software_version = newSwVersion;
	uC__nvsUpdate_u64("otaSwVersion", nvsHandle_otaSwVersion, &otaSwVersion_NVS, newSwVersion);
}


void FOTA__init (void)
{

	uC__nvsInitStorage("otaSwVersion", &nvsHandle_otaSwVersion);
	uC__nvsInitStorage("otaTrigSwUpdate", &nvsHandle_otaTrigSwUpdate);

#if 0
	ota_config.current_software_version = uC__nvsRead_u64("otaSwVersion", nvsHandle_otaSwVersion, &otaSwVersion_NVS);
	ota_config.trigger_software_update = uC__nvsRead_u8("otaTrigSwUpdate", nvsHandle_otaTrigSwUpdate, &otaTrigSwUpdate_NVS);
	uC__nvsUpdate_u8("otaTrigSwUpdate", nvsHandle_otaTrigSwUpdate, &otaTrigSwUpdate_NVS, FALSE);

	ota_config.server_host_name = OTA_SERVER_HOST_NAME;
	ota_config.server_port = "443";
	strncpy(ota_config.server_metadata_path, OTA_SERVER_METADATA_PATH, sizeof(ota_config.server_metadata_path) / sizeof(char));
	bzero(ota_config.server_firmware_path, sizeof(ota_config.server_firmware_path) / sizeof(char));
	ota_config.server_root_ca_public_key_pem = server_root_ca_public_key_pem;
	ota_config.peer_public_key_pem = peer_public_key_pem;
	ota_config.polling_interval_s = OTA_POLLING_INTERVAL_S;
	ota_config.auto_reboot = OTA_AUTO_REBOOT;

	iap_https_init(&ota_config);
#endif

	printf("OTA__init done\n");

	/* display current SW version and compile time */
	printf("\nCurrent SW version: %llu\n", FOTA__getCurrentSwVersion());
	printf("Compile Date: %s\n", __DATE__);
	printf("Compile Time: %s\n\n", __TIME__);
}

extern void ota_example_task(void *pvParameter);


void FOTA__enable (void)
{
	if (Wifi__isConnected())
	{
		FOTA__runBeforeSwUpdate();
		xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
	}
}


void FOTA__disable (void)
{
	FOTA__runAfterSwUpdate();
	//iap_https_stopTask();
}


void FOTA__runBeforeSwUpdate (void)
{
	IRMP__disable();
}


void FOTA__runAfterSwUpdate (void)
{
	IRMP__enable();
}


void FOTA__triggerSwUpdate (void)
{
	uC__nvsUpdate_u8("otaTrigSwUpdate", nvsHandle_otaTrigSwUpdate, &otaTrigSwUpdate_NVS, TRUE);
	uC__triggerSwReset();
}


uint8_t FOTA__isSwUpdateTriggered (void)
{
	return ota_config.trigger_software_update;
}


/*read buffer by byte still delim ,return read bytes counts*/
static int read_until(char *buffer, char delim, int len)
{
	//  /*TODO: delim check,buffer check,further: do an buffer length limited*/
	int i = 0;

	while (buffer[i] != delim && i < len)
	{
		++i;
	}

	return i + 1;
}


/* resolve a packet from http socket
 * return true if packet including \r\n\r\n that means http packet header finished,start to receive packet body
 * otherwise return false
 * */
static bool read_past_http_header(char text[], int total_len, esp_ota_handle_t update_handle)
{
	/* i means current position */
	int i = 0, i_read_len = 0;

	while (text[i] != 0 && i < total_len)
	{
		i_read_len = read_until(&text[i], '\n', total_len);
		// if we resolve \r\n line,we think packet header is finished

		if (i_read_len == 2)
		{
			int i_write_len = total_len - (i + 2);

			memset(ota_write_data, 0, BUFFSIZE);

			/*copy first http packet body to write buffer*/
			memcpy(ota_write_data, &(text[i + 2]), i_write_len);

			esp_err_t err = esp_ota_write(update_handle,
					(const void *) ota_write_data, i_write_len);
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
				return false;
			}
			else
			{
				ESP_LOGI(TAG, "esp_ota_write header OK");
				binary_file_length += i_write_len;
			}

			return true;
		}

		i += i_read_len;
	}

	return false;
}


static bool connect_to_http_server()
{
	ESP_LOGI(TAG, "Server IP: %s Server Port:%s", FOTA_SERVER_HOST_NAME, EXAMPLE_SERVER_PORT);

	int http_connect_flag = -1;
	struct sockaddr_in sock_info;

	socket_id = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_id == -1)
	{
		ESP_LOGE(TAG, "Create socket failed!");
		return false;
	}

	// set connect info
	memset(&sock_info, 0, sizeof(struct sockaddr_in));
	sock_info.sin_family = AF_INET;
	sock_info.sin_addr.s_addr = inet_addr(FOTA_SERVER_HOST_NAME);
	sock_info.sin_port = htons(atoi(FOTA_SERVER_PORT));

	// connect to http server
	http_connect_flag = connect(socket_id, (struct sockaddr *) &sock_info, sizeof(sock_info));

	if (http_connect_flag == -1)
	{
		ESP_LOGE(TAG, "Connect to server failed! errno=%d", errno);
		close(socket_id);
		return false;
	}
	else
	{
		ESP_LOGI(TAG, "Connected to server");
		return true;
	}

	return false;
}


static void task_fatal_error (void)
{
	ESP_LOGE(TAG, "Exiting task due to fatal error...");
	close(socket_id);
	(void) vTaskDelete(NULL);

	fotaState = FOTA_STATE_ERROR;
}


static void task_delete (void)
{
	ESP_LOGE(TAG, "Delete task...");
	close(socket_id);
	(void) vTaskDelete(NULL);
}


void ota_example_task(void *pvParameter)
{
	esp_err_t err;
	/* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
	esp_ota_handle_t update_handle = 0;
	const esp_partition_t *update_partition = NULL;

	const esp_partition_t *configured = esp_ota_get_boot_partition();
	const esp_partition_t *running = esp_ota_get_running_partition();

	if (configured != running)
	{
		ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
		ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
	}

	ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);

	fotaState = FOTA_STATE_CONNECTION_IN_PROGRESS;

	if (connect_to_http_server())
	{
		ESP_LOGI(TAG, "Connected to http server");
	}
	else
	{
		ESP_LOGE(TAG, "Connect to http server failed!");
		task_fatal_error();
	}

	/*send GET request to http server*/
	const char *GET_FORMAT = "GET %s HTTP/1.0\r\n"
			"Host: %s:%s\r\n"
			"User-Agent: esp-idf/1.0 esp32\r\n\r\n";

	char *http_request = NULL;
	int get_len = asprintf(&http_request, GET_FORMAT, FOTA_BINARY_FILE_NAME, FOTA_SERVER_HOST_NAME, FOTA_SERVER_PORT);

	if (get_len < 0)
	{
		ESP_LOGE(TAG, "Failed to allocate memory for GET request buffer");
		task_fatal_error();
	}

	int res = send(socket_id, http_request, get_len, 0);
	free(http_request);

	if (res < 0)
	{
		ESP_LOGE(TAG, "Send GET request to server failed");
		task_fatal_error();
	}
	else
	{
		ESP_LOGI(TAG, "Send GET request to server succeeded");
	}

	fotaState = FOTA_STATE_UPDATE_IN_PROGRESS;

	update_partition = esp_ota_get_next_update_partition(NULL);
	ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",	update_partition->subtype, update_partition->address);
	assert(update_partition != NULL);

	err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
		task_fatal_error();
	}

	ESP_LOGI(TAG, "esp_ota_begin succeeded");

	bool resp_body_start = false, flag = true;

	/* deal with all receive packet */
	while (flag)
	{
		memset(text, 0, TEXT_BUFFSIZE);
		memset(ota_write_data, 0, BUFFSIZE);

		int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);

		if (buff_len < 0)
		{
			/* receive error */
			ESP_LOGE(TAG, "Error: receive data error! errno=%d", errno);
			task_fatal_error();
		}
		else if ((buff_len > 0) && (!resp_body_start))
		{
			/* deal with response header */
			memcpy(ota_write_data, text, buff_len);
			resp_body_start = read_past_http_header(text, buff_len,	update_handle);
		}
		else if ((buff_len > 0) && (resp_body_start))
		{
			/* deal with response body */
			memcpy(ota_write_data, text, buff_len);
			err = esp_ota_write(update_handle, (const void *) ota_write_data, buff_len);

			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
				task_fatal_error();
			}

			binary_file_length += buff_len;
			ESP_LOGI(TAG, "Have written image length %d", binary_file_length);
			ESP_LOGI(TAG, "Buffer left %d", buff_len);
		}
		else if (buff_len == 0)
		{
			/* packet over */
			flag = false;
			ESP_LOGI(TAG, "Connection closed, all packets received");
			close(socket_id);
		}
		else
		{
			ESP_LOGE(TAG, "Unexpected recv result");
		}
	}

	ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

	if (esp_ota_end(update_handle) != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_ota_end failed!");
		task_fatal_error();
	}

	err = esp_ota_set_boot_partition(update_partition);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
		task_fatal_error();
	}

	ESP_LOGI(TAG, "SW updated successfully!");
	FOTA__runAfterSwUpdate();
	fotaState = FOTA_STATE_UPDADE_FINISHED;
	task_delete();

	return;
}

