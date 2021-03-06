/*
 * LEDMatrix.c
 *
 *  Created on: 20.01.2013
 *      Author: Jean-Martin George
 */

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "LEDMATRIX"

#include "LEDMatrix.h"
#include "esp_log.h"


#ifdef LED_MATRIX_SIZE_LIN
#ifdef LED_MATRIX_SIZE_COL


#if (LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
static uint8_t ledOrder;
static nvs_handle nvsHandle_ledOrder;
static uint8_t ledOrder_NVS;
#endif


void LEDMatrix__setRGBColor (uint8_t line, uint8_t column, RGB_Color_t color)
{
	uint16_t ledPosition;

	LEDMatrix__applyDotCorrection(&color, line, column);

	if (LEDMatrix__getLedOrder() == LED_ORDER__ZIG_ZAG)
	{
		if (((line % 2) == 0) && (column <= LED_MATRIX_SIZE_COL))
		{
			/* column overflow allowed for Qlocktwo -> not considered here */
			column = (LED_MATRIX_SIZE_COL + 1) - column;
		}
	}

	ledPosition = LED_MATRIX_SIZE_COL * (line - 1) + (column - 1);

#if (LED_TYPE == LED_TYPE_WS2801)
	WS2801__setRGBForLED(color, ledPosition);
#endif
#if (LED_TYPE == LED_TYPE_WS2812)
	WS2812__setRGBForLED(color, ledPosition);
#endif
#if (LED_TYPE == LED_TYPE_APA102)
	APA102__setRGBForLED(color, ledPosition);
#endif
}


void LEDMatrix__setRGBColorForMatrix (RGB_Color_t color)
{
	uint16_t linIt, colIt;
	uint16_t newColumn;
	uint16_t ledPosition = 0;
	RGB_Color_t colorCorrected;

#if (LED_TYPE == LED_TYPE_WS2801)
	WS2801__setRGBForAllLEDs(color);
#endif
#if (LED_TYPE == LED_TYPE_WS2812)
	WS2812__setRGBForAllLEDs(color);
#endif
#if (LED_TYPE == LED_TYPE_APA102)

	for (colIt = 1; colIt <= LED_MATRIX_SIZE_COL; colIt++)
	{
		for (linIt = 1; linIt <= LED_MATRIX_SIZE_LIN; linIt++)
		{
			colorCorrected = color;
			LEDMatrix__applyDotCorrection(&colorCorrected, linIt, colIt);

			if (LEDMatrix__getLedOrder() == LED_ORDER__ZIG_ZAG)
			{
				if ((linIt % 2) == 0)
				{
					newColumn = (LED_MATRIX_SIZE_COL + 1) - colIt;
				}
				else
				{
					newColumn = colIt;
				}
			}
			else
			{
				newColumn = colIt;
			}

			ledPosition = LED_MATRIX_SIZE_COL * (linIt - 1) + (newColumn - 1);
			APA102__setRGBForLED(colorCorrected, ledPosition);
		}
	}
#endif
}


void LEDMatrix__clearMatrix (void)
{
#if (LED_TYPE == LED_TYPE_WS2801)
	WS2801__resetAllLEDs();
#endif
#if (LED_TYPE == LED_TYPE_WS2812)
	WS2812__resetAllLEDs();
#endif
#if (LED_TYPE == LED_TYPE_APA102)
	APA102__resetAllLEDs();
#endif
}


void LEDMatrix__enableUpdate (void)
{
#if (LED_TYPE == LED_TYPE_WS2801)
	WS2801__enableUpdate(TRUE);
#endif
#if (LED_TYPE == LED_TYPE_WS2812)
	WS2812__enableUpdate(TRUE);
#endif
#if (LED_TYPE == LED_TYPE_APA102)
	APA102__enableUpdate(TRUE);
#endif
}


void LEDMatrix__disableUpdate (void)
{
#if (LED_TYPE == LED_TYPE_WS2801)
	WS2801__disableUpdate(FALSE);
#endif
#if (LED_TYPE == LED_TYPE_WS2812)
	WS2812__disableUpdate(FALSE);
#endif
#if (LED_TYPE == LED_TYPE_APA102)
	APA102__disableUpdate(FALSE);
#endif
}


void LEDMatrix__toggleRGBLedOrder (void)
{
#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
#if (LED_TYPE == LED_TYPE_WS2801)
	WS2801__toggleRGBLedOrder();
#endif
#if (LED_TYPE == LED_TYPE_WS2812)
	WS2812__toggleRGBLedOrder();
#endif
#if (LED_TYPE == LED_TYPE_APA102)
	APA102__toggleRGBLedOrder();
#endif
#endif
}


void LEDMatrix__toggleLedOrder (void)
{
#if (LED_ORDER == LED_ORDER__CONFIGURABLE)
	if (ledOrder == LED_ORDER__LEFT_2_RIGHT)
	{
		ledOrder = LED_ORDER__ZIG_ZAG;
	}
	else
	{
		ledOrder = LED_ORDER__LEFT_2_RIGHT;
	}

	uC__nvsUpdate_u8("ledOrder", nvsHandle_ledOrder, &ledOrder_NVS, ledOrder);
#endif
}


uint8_t LEDMatrix__getLedOrder (void)
{
#if (LED_ORDER == LED_ORDER__LEFT_2_RIGHT)
	return LED_ORDER__LEFT_2_RIGHT;
#elif (LED_ORDER == LED_ORDER__STRAIGHT_FORWARD)
	return LED_ORDER__ZIG_ZAG;
#else
	return ledOrder;
#endif
}


void LEDMatrix__init (void)
{
	LEDMatrix__clearMatrix();
#if (LED_ORDER == LED_ORDER__CONFIGURABLE)
	uC__nvsInitStorage("ledOrder", &nvsHandle_ledOrder);

	ledOrder = uC__nvsRead_u8("ledOrder", nvsHandle_ledOrder, &ledOrder_NVS);

	if (	(ledOrder != LED_ORDER__LEFT_2_RIGHT)
		&& 	(ledOrder != LED_ORDER__ZIG_ZAG)
		)
	{
		ledOrder = LED_ORDER__ZIG_ZAG;
		uC__nvsUpdate_u8("ledOrder", nvsHandle_ledOrder, &ledOrder_NVS, ledOrder);
	}
#endif

	ESP_LOGI(LOG_TAG, "LEDMatrix__init done");
}

#endif
#endif
