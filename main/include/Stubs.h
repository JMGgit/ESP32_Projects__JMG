/*
 * Stubs.h
 *
 *  Created on: 22.02.2017
 *      Author: Jean-Martin George
 */

#ifndef STUBS_H_
#define STUBS_H_

#include "stdint.h"


/* global project configuration */
#define PROJECT								PROJECT__LED_TABLE
#define LEDTABLE_REVISION					LEDTABLE_REVISION_2


/* stubs from  ATMega projects */
#define PROGMEM
#define EEMEM
#define pgm_read_byte(x) (*(x))
#define FailureMemory__x10()
#define uC__getTaskIncrement() (1)
#define setInput(ddr, pin)
#define isLow(port, pin)
#define isHigh(port, pin)
#define setOutput(ddr, pin)
#define setLow(port, pin)
#define setHigh(port, pin)
#define toggle(port, pin)

#endif /* STUBS_H_ */
