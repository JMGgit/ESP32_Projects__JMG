/*
 * Ota.h
 *
 *  Created on: 09.11.2015
 *      Author: Jean-Martin George
 */


#ifndef OTA_H_
#define OTA_H_


#include "Main_Types.h"
#include "Main_Config.h"
#include "uC.h"
#include "iap_https.h"
#include "OTA_Cfg.h"


#define OTA_POLLING_INTERVAL_S    1
#define OTA_AUTO_REBOOT           0

typedef struct
{
    // Version number of the running firmware image.
    uint64_t current_software_version;

    // Version number of the server firmware image.
    uint64_t server_software_version;

    // Force SW update
    uint8_t trigger_software_update;

} OTA_config_t;

typedef enum
{
	OTA_STATE_IDLE = 0,
	OTA_STATE_DOWNLOAD_IN_PROGRESS,
	OTA_STATE_UPDATE_IN_PROGRESS,
	OTA_STATE_UPDADE_FINISHED
} OTA_State_t;


void OTA__init (void);

void OTA__enable (void);
void OTA__disable (void);

OTA_State_t OTA__getCurrentState (void);

uint64_t OTA__getCurrentSwVersion (void);
void OTA__setCurrentSwVersion (uint64_t newSwVersion);

void OTA__runBeforeSwUpdate (void);
void OTA__runAfterSwUpdate (void);

void OTA__triggerSwUpdate (void);
uint8_t OTA__isSwUpdateTriggered (void);


#endif /* OTA_H_ */
