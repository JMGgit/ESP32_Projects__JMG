/*
 * IRMP.h
 *
 *  Created on: 23.12.2014
 *      Author: Jean-Martin George
 */

#ifndef SRC_DRIVERS_IRMP_IRMP_APPL_H_
#define SRC_DRIVERS_IRMP_IRMP_APPL_H_

//#include <avr/io.h>
#include "Main_Types.h"
#include "Main_Config.h"
//#include "Drivers_Config.h"
#include "irmp.h"
#include "uC.h"


#if (BUTTONS_IRMP != BUTTONS_IRMP_OFF)

void IRMP__init (void);
uint8_t IRMP__readData (uint16_t address, uint8_t *data, uint8_t dataLength, uint8_t *repeat);
void IRMP__mainFunction (void* param);
void IRMP__disable (void);
void IRMP__enable (void);

#endif

#endif /* SRC_DRIVERS_IRMP_IRMP_APPL_H_ */
