/*
 * Ota.c
 *
 *  Created on: 24.05.2017
 *      Author: Jean-Martin George
 */


#include "OTA.h"
#include "IRMP_Appl.h"


static const char *server_root_ca_public_key_pem = OTA_SERVER_ROOT_CA_PEM;
static const char *peer_public_key_pem = OTA_PEER_PEM;

static iap_https_config_t ota_config;

static uint8_t otaSwVersion_NVS;
static nvs_handle nvsHandle_otaSwVersion;

static uint8_t otaTrigSwUpdate_NVS;
static nvs_handle nvsHandle_otaTrigSwUpdate;


uint8_t OTA__getCurrentSwVersion (void)
{
	return ota_config.current_software_version;
}


void OTA__setCurrentSwVersion (uint8_t newSwVersion)
{
	ota_config.current_software_version = newSwVersion;
	uC__nvsUpdateByte("otaSwVersion", nvsHandle_otaSwVersion, &otaSwVersion_NVS, newSwVersion);
}


void OTA__init (void)
{
	uC__nvsInitStorage("otaSwVersion", &nvsHandle_otaSwVersion);
	uC__nvsInitStorage("otaTrigSwUpdate", &nvsHandle_otaTrigSwUpdate);

	ota_config.current_software_version = uC__nvsReadByte("otaSwVersion", nvsHandle_otaSwVersion, &otaSwVersion_NVS);
	ota_config.trigger_software_update = uC__nvsReadByte("otaTrigSwUpdate", nvsHandle_otaTrigSwUpdate, &otaTrigSwUpdate_NVS);
	uC__nvsUpdateByte("otaTrigSwUpdate", nvsHandle_otaTrigSwUpdate, &otaTrigSwUpdate_NVS, FALSE);

	ota_config.server_host_name = OTA_SERVER_HOST_NAME;
	ota_config.server_port = "443";
	strncpy(ota_config.server_metadata_path, OTA_SERVER_METADATA_PATH, sizeof(ota_config.server_metadata_path) / sizeof(char));
	bzero(ota_config.server_firmware_path, sizeof(ota_config.server_firmware_path) / sizeof(char));
	ota_config.server_root_ca_public_key_pem = server_root_ca_public_key_pem;
	ota_config.peer_public_key_pem = peer_public_key_pem;
	ota_config.polling_interval_s = OTA_POLLING_INTERVAL_S;
	ota_config.auto_reboot = OTA_AUTO_REBOOT;

	iap_https_init(&ota_config);
}


void OTA__enable (void)
{
	iap_https_startTask();
	iap_https_check_now();
}


void OTA__disable (void)
{
	iap_https_stopTask();
}


OTA_State_t OTA__getCurrentState (void)
{
	OTA_State_t otaState;

	if (iap_https_new_firmware_installed())
	{
		otaState = OTA_STATE_UPDADE_FINISHED;
	}
	else if (iap_https_update_in_progress())
	{
		otaState = OTA_STATE_UPDATE_IN_PROGRESS;
	}
	else if (iap_https_download_in_progress())
	{
		otaState = OTA_STATE_DOWNLOAD_IN_PROGRESS;
	}
	else
	{
		otaState = OTA_STATE_IDLE;
	}

	return otaState;
}


void OTA__runBeforeSwUpdate (void)
{
	IRMP__disable();
}


void OTA__runAfterSwUpdate (void)
{
	IRMP__enable();
}


void OTA__triggerSwUpdate (void)
{
	uC__nvsUpdateByte("otaTrigSwUpdate", nvsHandle_otaTrigSwUpdate, &otaTrigSwUpdate_NVS, TRUE);
	uC__triggerSwReset();
}


uint8_t OTA__isSwUpdateTriggered (void)
{
	return ota_config.trigger_software_update;
}
