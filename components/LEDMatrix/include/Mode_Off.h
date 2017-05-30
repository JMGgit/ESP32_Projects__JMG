/*
 * Mode_Off.h
 *
 *  Created on: 07.04.2016
 *      Author: JMG
 */

#ifndef MODE_OFF_H_
#define MODE_OFF_H_


#include "Main_Types.h"
#include "Main_Config.h"
#include "Buttons.h"
#include "LEDMatrix.h"
#if (OTA_SW_UPDATE == OTA_SW_UPDATE_ON)
#include "OTA.h"
#endif

void Off__x10 (void);

#endif /* MODE_QTWO_H_ */
