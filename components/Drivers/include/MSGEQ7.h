/*
 * MSGEQ7.h
 *
 *  Created on: 28.03.2017
 *      Author: Jean-Martin George
 */

#ifndef MSGEQ7_H_
#define MSGEQ7_H_


#include "uC.h"

#if (EQUALIZER == EQUALIZER_MSGEQ7)

void MSGEQ7__readValues (uint16_t *adcValues); /* length of adcValues buffer: 7 */
void MSGEQ7__mainFunction (void *param);
void MSGEQ7__init (void);

#define MSGEQ7DEBUG 	FALSE

#endif

#endif /* MSGEQ7_H_ */
