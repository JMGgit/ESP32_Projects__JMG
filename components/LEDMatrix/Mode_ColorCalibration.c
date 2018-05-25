/*
 * Mode_ColorCalibration.c
 *
 *  Created on: 24.05.2018
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "COLORCAL"

#include "Mode_ColorCalibration.h"
#include "esp_log.h"
#include "Buttons.h"


#define TIMER_COLOR_SELECT	100

typedef enum
{
	STATE_SELECT = 0,
	STATE_REDCAL,
	STATE_GREENCAL,
	STATE_BLUECAL
} COLORCALIBRATIONSTATE_N;


static uint16_t currentLedLine = 1;
static uint16_t currentLedCol = 1;
static COLORCALIBRATIONSTATE_N state = STATE_SELECT;
static uint8_t timerColorSelect;


void ColorCalibration__init (void)
{
	ESP_LOGI(LOG_TAG, "Entering LED calibration mode");
	state = STATE_SELECT;
	/* don't reset currentLed to be able to continue quickly the calibration */
}


void ColorCalibration__x10 (void)
{
	static uint8_t dummyValue = 255;

	LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(255, 255, 255));

	switch (state)
	{

	case STATE_SELECT:
	{
		if ((Buttons__isPressedOnce(&buttonLeft)) && (currentLedCol > 1))
		{
			currentLedCol--;
		}

		if ((Buttons__isPressedOnce(&buttonRight)) && (currentLedCol < LED_MATRIX_SIZE_COL))
		{
			currentLedCol++;
		}

		if ((Buttons__isPressedOnce(&buttonUp)) && (currentLedLine > 1))
		{
			currentLedLine--;
		}

		if ((Buttons__isPressedOnce(&buttonDown)) && (currentLedLine < LED_MATRIX_SIZE_LIN))
		{
			currentLedLine++;
		}

		if (Buttons__isPressedOnce(&buttonFunc1))
		{
			ESP_LOGI(LOG_TAG, "Calibrating LED: line %d, col %d)", currentLedLine, currentLedCol);
			state = STATE_REDCAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc2))
		{
			ESP_LOGI(LOG_TAG, "Calibrating LED: line %d, col %d)", currentLedLine, currentLedCol);
			state = STATE_GREENCAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc3))
		{
			ESP_LOGI(LOG_TAG, "Calibrating LED: line %d, col %d)", currentLedLine, currentLedCol);
			state = STATE_BLUECAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		LEDMatrix__setRGBColor(currentLedLine, currentLedCol, LEDMatrix__getRGBColorFromComponents(0, 0, 0));

		break;
	}

	case STATE_REDCAL:
	{
		if (timerColorSelect > 0)
		{
			timerColorSelect--;
			LEDMatrix__setRGBColor(currentLedLine, currentLedCol, LEDMatrix__getRGBColorFromComponents(255, 0, 0));
		}
		else
		{
			LEDMatrix__setRGBColor(currentLedLine, currentLedCol, LEDMatrix__getRGBColorFromComponents(255, 255, 255));
		}

		if ((Buttons__isPressedOnce(&buttonUp)) && ((dummyValue <= (255 - COLOR_CALIBRATION_STEP))))
		{
			dummyValue = dummyValue + COLOR_CALIBRATION_STEP;
			ESP_LOGI(LOG_TAG, "Current values: red %d, green %d, blue %d)", dummyValue, 255, 255);
		}

		if ((Buttons__isPressedOnce(&buttonDown)) && (dummyValue >= COLOR_CALIBRATION_STEP))
		{
			dummyValue = dummyValue - COLOR_CALIBRATION_STEP;
			ESP_LOGI(LOG_TAG, "Current values: red %d, green %d, blue %d)", dummyValue, 255, 255);
		}

		if (Buttons__isPressedOnce(&buttonFunc1))
		{
			state = STATE_SELECT;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc2))
		{
			state = STATE_GREENCAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc3))
		{
			state = STATE_BLUECAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		break;
	}

	case STATE_GREENCAL:
	{
		if (timerColorSelect > 0)
		{
			timerColorSelect--;
			LEDMatrix__setRGBColor(currentLedLine, currentLedCol, LEDMatrix__getRGBColorFromComponents(0, 255, 0));
		}
		else
		{
			LEDMatrix__setRGBColor(currentLedLine, currentLedCol, LEDMatrix__getRGBColorFromComponents(255, 255, 255));
		}

		if ((Buttons__isPressedOnce(&buttonUp)) && ((dummyValue <= (255 - COLOR_CALIBRATION_STEP))))
		{
			dummyValue = dummyValue + COLOR_CALIBRATION_STEP;
			ESP_LOGI(LOG_TAG, "Current values: red %d, green %d, blue %d)", 255, dummyValue, 255);
		}

		if ((Buttons__isPressedOnce(&buttonDown)) && (dummyValue >= COLOR_CALIBRATION_STEP))
		{
			dummyValue = dummyValue - COLOR_CALIBRATION_STEP;
			ESP_LOGI(LOG_TAG, "Current values: red %d, green %d, blue %d)", 255, dummyValue, 255);
		}

		if (Buttons__isPressedOnce(&buttonFunc1))
		{
			state = STATE_REDCAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc2))
		{
			state = STATE_SELECT;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc3))
		{
			state = STATE_BLUECAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		break;
	}

	case STATE_BLUECAL:
	{
		if (timerColorSelect > 0)
		{
			timerColorSelect--;
			LEDMatrix__setRGBColor(currentLedLine, currentLedCol, LEDMatrix__getRGBColorFromComponents(0, 0, 255));
		}
		else
		{
			LEDMatrix__setRGBColor(currentLedLine, currentLedCol, LEDMatrix__getRGBColorFromComponents(255, 255, 255));
		}

		if ((Buttons__isPressedOnce(&buttonUp)) && ((dummyValue <= (255 - COLOR_CALIBRATION_STEP))))
		{
			dummyValue = dummyValue + COLOR_CALIBRATION_STEP;
			ESP_LOGI(LOG_TAG, "Current values: red %d, green %d, blue %d)", 255, 255, dummyValue);
		}

		if ((Buttons__isPressedOnce(&buttonDown)) && (dummyValue >= COLOR_CALIBRATION_STEP))
		{
			dummyValue = dummyValue - COLOR_CALIBRATION_STEP;
			ESP_LOGI(LOG_TAG, "Current values: red %d, green %d, blue %d)", 255, 255, dummyValue);
		}

		if (Buttons__isPressedOnce(&buttonFunc1))
		{
			state = STATE_REDCAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc2))
		{
			state = STATE_GREENCAL;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		if (Buttons__isPressedOnce(&buttonFunc3))
		{
			state = STATE_SELECT;
			timerColorSelect = TIMER_COLOR_SELECT;
		}

		break;
	}

	}
}
