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

} FOTA_config_t;

typedef enum
{
	FOTA_STATE_IDLE = 0,
	FOTA_STATE_ERROR,
	FOTA_STATE_CONNECTION_IN_PROGRESS,
	FOTA_STATE_UPDATE_IN_PROGRESS,
	FOTA_STATE_UPDADE_FINISHED
} OTA_State_t;


void FOTA__init (void);

void FOTA__enable (void);
void FOTA__disable (void);

OTA_State_t FOTA__getCurrentState (void);

uint64_t FOTA__getCurrentSwVersion (void);
void FOTA__setCurrentSwVersion (uint64_t newSwVersion);

void FOTA__runBeforeSwUpdate (void);
void FOTA__runAfterSwUpdate (void);

void FOTA__triggerSwUpdate (void);
uint8_t FOTA__isSwUpdateTriggered (void);


#endif /* OTA_H_ */
