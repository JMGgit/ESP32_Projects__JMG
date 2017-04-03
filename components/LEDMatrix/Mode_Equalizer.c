/*
 * Mode_Equalizer.c
 *
 *  Created on: 03.04.2017
 *      Author: Jean-Martin George
 */


#include "Mode_Equalizer.h"

#define ADCVALUES_NB	7


void Equalizer__init (void)
{
#if (EQUALIZER == EQUALIZER_MSGEQ7)
	MSGEQ7__init();
#endif
}


void Equalizer__x10 (void)
{
	uint16_t adcValues[ADCVALUES_NB];

#if (EQUALIZER == EQUALIZER_MSGEQ7)
	MSGEQ7__readValues(&adcValues[0]);
#endif



}


