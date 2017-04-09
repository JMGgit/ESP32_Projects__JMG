/*
 * Mode_Equalizer.c
 *
 *  Created on: 03.04.2017
 *      Author: Jean-Martin George
 */


#include "Mode_Equalizer.h"
#include "Mode_ColorBlending.h"

#define ADCVALUES_NB			7
#define ADC_MIN_VALUE_IN		70
#define ADC_MAX_VALUE_IN		4095
#define ADC_MIN_VALUE_OUT		0
#define ADC_MAX_VALUE_OUT		(LED_MATRIX_SIZE_LIN - 1)

#define UPDATE_TIME				20
#define UPDATE_TIME_FAST		1
#define COLOR_STEP				2


void Equalizer__init (void)
{
#if (EQUALIZER == EQUALIZER_MSGEQ7)
	MSGEQ7__init();
#endif
}


void Equalizer__x10 (void)
{
	uint16_t adcValues[ADCVALUES_NB];
	RGB_Color_t eqColor;
	uint8_t itCol, itLin;

#if (EQUALIZER == EQUALIZER_MSGEQ7)
	MSGEQ7__readValueswithResolution(&adcValues[0], ADC_MIN_VALUE_IN, ADC_MAX_VALUE_IN, ADC_MIN_VALUE_OUT, ADC_MAX_VALUE_OUT);
#endif

	ColorBlending__calcCurrentColor(UPDATE_TIME, COLOR_STEP);
	eqColor = ColorBlending__getCurrentColor();

	LEDMatrix__clearMatrix();

	for (itCol = 0; itCol < LED_MATRIX_SIZE_COL; itCol++)
	{
		for (itLin = 0; itLin < LED_MATRIX_SIZE_LIN; itLin++)
		{
			if (itCol < ADCVALUES_NB)
			{
				if (itLin < adcValues[itCol % ADCVALUES_NB])
				{
					LEDMatrix__setRGBColor(itLin + 1, itCol + 1, eqColor);
				}
			}
		}
	}
}
