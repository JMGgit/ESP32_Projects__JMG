/*
 * Mode_Off.h
 *
 *  Created on: 07.04.2016
 *      Author: Jean-Martin George
 */

#ifndef MODE_OFF_H_
#define MODE_OFF_H_


#include "Main_Types.h"
#include "Main_Config.h"
#include "Buttons.h"
#include "LEDMatrix.h"
#if (FOTA_SW_UPDATE == FOTA_SW_UPDATE_ON)
#include "FOTA.h"
#endif

void Off__x10 (void);

#endif /* MODE_QTWO_H_ */
