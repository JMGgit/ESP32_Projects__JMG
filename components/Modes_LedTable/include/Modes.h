/*
 * Modes.h
 *
 *  Created on: 09.11.2015
 *      Author: Jean-Martin George
 */

#ifndef MODES_H_
#define MODES_H_

#include "Main_Config.h"
#if (PROJECT == PROJECT__TEMPERATURE_LOGGER)
#include "Modes_TemperatureLogger.h"
#endif
#if (PROJECT == PROJECT__QLOCKTWO)
#include "Modes_Qlocktwo.h"
#endif
#if (PROJECT == PROJECT__LED_TABLE)
#include "Modes_LedTable.h"
#endif
#if (PROJECT == PROJECT__LED_MIRROR)
#include "Modes_LedMirror.h"
#endif
#if (PROJECT == PROJECT__LED_FRAME)
#include "Modes_LedFrame.h"
#endif


/* public functions */
void Modes__init (void);
void Modes__x10 (void);
#if (PROJECT != PROJECT__TEMPERATURE_LOGGER)
void Modes__setMode (Mode_t mode, uint8_t transition);
#endif
Mode_t Modes__getMode (void);
void Modes__Start (void);
uint8_t Modes__getTaskIncrement (void);


#endif /* MODES_H_ */
