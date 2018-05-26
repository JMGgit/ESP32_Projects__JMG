/*
 * LEDMatrix_DotCorrection.h
 *
 *  Created on: 30.11.2013
 *      Author: Jean-Martin George
 */

#ifndef LEDMATRIX_DOTCORRECTION_H_
#define LEDMATRIX_DOTCORRECTION_H_

#include "Main_Types.h"
#include "Main_Config.h"
#include "RGB_Tables.h"


uint8_t LEDMatrixDotCorrectionArray[3 * LEDS_CHANNELS];


static inline void LEDMatrix__setDotCorrectionForLed (uint8_t line, uint8_t column, uint8_t valueRed, uint8_t valueGreen, uint8_t valueBlue)
{
	LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1))] = valueRed;
	LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 1] = valueGreen;
	LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 2] = valueBlue;
}


static inline void LEDMatrix__setDotCorrectionForLed_Red (uint8_t line, uint8_t column, uint8_t value)
{
	LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1))] = value;
}


static inline void LEDMatrix__setDotCorrectionForLed_Green (uint8_t line, uint8_t column, uint8_t value)
{
	LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 1] = value;
}


static inline void LEDMatrix__setDotCorrectionForLed_Blue (uint8_t line, uint8_t column, uint8_t value)
{
	LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 2] = value;
}


static inline uint8_t LEDMatrix__getDotCorrectionForLed_Red (uint8_t line, uint8_t column)
{
	return LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1))];
}


static inline uint8_t LEDMatrix__getDotCorrectionForLed_Green (uint8_t line, uint8_t column)
{
	return LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 1];
}


static inline uint8_t LEDMatrix__getDotCorrectionForLed_Blue (uint8_t line, uint8_t column)
{
	return LEDMatrixDotCorrectionArray[3 * ((line - 1) * LED_MATRIX_SIZE_COL + (column - 1)) + 2];
}


void LEDMatrix__applyDotCorrection (RGB_Color_t* color, uint8_t line, uint8_t column);

#endif /* LEDMATRIX_DOTCORRECTION_H_ */
