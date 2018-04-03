/*
 * FOTA.h
 *
 *  Created on: 09.11.2015
 *      Author: Jean-Martin George
 */


#ifndef FOTA_H_
#define FOTA_H_


#include "Main_Types.h"
#include "Main_Config.h"
#include "FOTA_Cfg.h"


typedef enum
{
	FOTA_STATE_IDLE = 0,
	FOTA_STATE_ERROR,
	FOTA_STATE_CONNECTION_IN_PROGRESS,
	FOTA_STATE_UPDATE_IN_PROGRESS,
	FOTA_STATE_UPDADE_FINISHED
} FOTA_State_t;


void FOTA__init (void);
void FOTA__mainFunction(void *param);

void FOTA__enable (void);
void FOTA__disable (void);

FOTA_State_t FOTA__getCurrentState (void);

uint64_t FOTA__getCurrentSwVersion (void);
void FOTA__setCurrentSwVersion (uint64_t newSwVersion);

void FOTA__runBeforeSwUpdate (void);
void FOTA__runAfterSwUpdate (void);

void FOTA__triggerSwUpdate (void);
uint8_t FOTA__isSwUpdateTriggered (void);


#endif /* FOTA_H_ */
