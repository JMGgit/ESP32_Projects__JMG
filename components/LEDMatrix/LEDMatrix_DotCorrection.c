/*
 * LEDMatrix_DotCorrection.c
 *
 *  Created on: 30.11.2013
 *      Author: Jean-Martin George
 */


#include "LEDMatrix_DotCorrection.h"


#ifdef LED_MATRIX_SIZE_LIN


uint8_t LEDMatrixDotCorrectionArray[LEDS_CHANNELS] = {
/*     |      1        |       2        |       3        |       4        |       5        |       6        |       7        |       8        |       9        |      10       | */
/* 1 */	100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
/* 2 */	100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
/* 3 */	100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
/* 4 */	100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
/* 5 */	100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
/* 6 */	100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
};


void LEDMatrix__applyDotCorrection (RGB_Color_t* color, uint8_t line, uint8_t column)
{
#if (LED_COLORS == LED_COLORS_PWM)
	color->red = PWM_Table_256[color->red];
	color->green = PWM_Table_256[color->green];
	color->blue = PWM_Table_256[color->blue];
#endif

	if ((line != 0) && (column != 0))
	{
		color->red = (color->red * LEDMatrix__getDotCorrectionForLed_Red(line, column)) / 100;
		color->green = (color->green * LEDMatrix__getDotCorrectionForLed_Green(line, column)) / 100;
		color->blue = (color->blue * LEDMatrix__getDotCorrectionForLed_Blue(line, column)) / 100;
	}
}

#endif
