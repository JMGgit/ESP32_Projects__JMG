/*
 * Ota.c
 *
 *  Created on: 24.05.2017
 *      Author: Jean-Martin George
 */


#include "OTA.h"
#include <string.h>


static const char *server_root_ca_public_key_pem = OTA_SERVER_ROOT_CA_PEM;
static const char *peer_public_key_pem = OTA_PEER_PEM;

static iap_https_config_t ota_config;


void OTA__init (void)
{
    ota_config.current_software_version = OTA_SOFTWARE_VERSION;
    ota_config.server_host_name = OTA_SERVER_HOST_NAME;
    ota_config.server_port = "443";
    strncpy(ota_config.server_metadata_path, OTA_SERVER_METADATA_PATH, sizeof(ota_config.server_metadata_path) / sizeof(char));
    bzero(ota_config.server_firmware_path, sizeof(ota_config.server_firmware_path) / sizeof(char));
    ota_config.server_root_ca_public_key_pem = server_root_ca_public_key_pem;
    ota_config.peer_public_key_pem = peer_public_key_pem;
    ota_config.polling_interval_s = OTA_POLLING_INTERVAL_S;
    ota_config.auto_reboot = OTA_AUTO_REBOOT;

    iap_https_init(&ota_config);

    // Immediately check if there's a new firmware image available.
    iap_https_check_now();
}
