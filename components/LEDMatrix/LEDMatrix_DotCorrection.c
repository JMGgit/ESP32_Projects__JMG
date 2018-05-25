/*
 * LEDMatrix_DotCorrection.c
 *
 *  Created on: 30.11.2013
 *      Author: Jean-Martin George
 */


#include "LEDMatrix_DotCorrection.h"


#ifdef LED_MATRIX_SIZE_LIN


uint8_t LEDMatrixDotCorrectionArray[3 * LEDS_CHANNELS] = {
/*     |      1        |       2        |       3        |       4        |       5        |       6        |       7        |       8        |       9        |      10       | */
/* 1 */	75,  100, 90,    90,  100, 100,   85,  100, 90,    85,  100, 90,    100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
/* 2 */	80,  100, 80,    100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
/* 3 */	80,  100, 80,    100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,   100, 100, 100,
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

	color->red = (color->red * LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1))]) / 100;
	color->green = (color->green * LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 1]) / 100;
	color->blue = (color->blue * LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 2]) / 100;
}

#endif
