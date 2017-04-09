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

 /* length of adcValues buffer: 7 */
void MSGEQ7__readValues (uint16_t *adcValues);
void MSGEQ7__readValueswithResolution (uint16_t *adcValues, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
void MSGEQ7__mainFunction (void *param);
void MSGEQ7__init (void);

#define MSGEQ7DEBUG 	FALSE

#endif

#endif /* MSGEQ7_H_ */
