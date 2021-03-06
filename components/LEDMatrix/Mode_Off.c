/*
 * Mode_Off.c
 *
 *  Created on: 07.04.2016
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "MODE_OFF"

#include "Mode_Off.h"
#include "Modes.h"
#include "esp_log.h"

static uint8_t firstCall = TRUE;

void Off__x10 (void)
{
	static uint8_t rgbConnectionTimer = 255;
	static uint8_t ledConnectionTimer = 255;
	static uint8_t startupTimer = 255;
	static uint8_t fotaTimer = 255;
	static uint8_t colorCalibrationTimer = 255;
#if (PROJECT == PROJECT__QLOCKTWO)
	static uint8_t langTimer = 255;
#endif

	if (firstCall)
	{
		LEDMatrix__clearMatrix();
		firstCall = FALSE;
	}
	else
	{
		LEDMatrix__disableUpdate();

		if (Buttons__isPressedOnce(&buttonOff))
		{
			Modes__Start();
			LEDMatrix__enableUpdate();
			firstCall = TRUE;
		}

		if (Buttons__isPressed(&buttonFunc1))
		{
			if (rgbConnectionTimer - uC__getTaskIncrement() > 0)
			{
				rgbConnectionTimer = rgbConnectionTimer - uC__getTaskIncrement();
			}
			else
			{
				rgbConnectionTimer = 255;
				LEDMatrix__toggleRGBLedOrder();
				uC__triggerSwReset();
			}
		}
		else
		{
			rgbConnectionTimer = 255;
		}

		if (Buttons__isPressed(&buttonFunc2))
		{
			if (ledConnectionTimer - uC__getTaskIncrement() > 0)
			{
				ledConnectionTimer = ledConnectionTimer - uC__getTaskIncrement();
			}
			else
			{
				ledConnectionTimer = 255;
				LEDMatrix__toggleLedOrder();
				uC__triggerSwReset();
			}
		}
		else
		{
			ledConnectionTimer = 255;
		}

		if (Buttons__isPressed(&buttonFunc3))
		{
			if (startupTimer - uC__getTaskIncrement() > 0)
			{
				startupTimer = startupTimer - uC__getTaskIncrement();
			}
			else
			{
				startupTimer = 255;
				Modes__toggleStartupMode();
				uC__triggerSwReset();
			}
		}
		else
		{
			startupTimer = 255;
		}

		if (Buttons__isPressedOnce(&buttonDown))
		{
			uC__triggerSwReset();
		}

#if (FOTA_SW_UPDATE == FOTA_SW_UPDATE_ON)
		if (Buttons__isPressed(&buttonUp))
		{
			if (fotaTimer - uC__getTaskIncrement() > 0)
			{
				fotaTimer = fotaTimer - uC__getTaskIncrement();
			}
			else
			{
				fotaTimer = 255;
				FOTA__toggleCyclicCheck();
				uC__triggerSwReset();
			}
		}
		else
		{
			fotaTimer = 255;
		}

		if (Buttons__isPressedOnce(&buttonLeft))
		{
			ESP_LOGI(LOG_TAG, "Switching to MODE__FOTA");
			Modes__setMode(MODE__FOTA, FALSE);
			LEDMatrix__enableUpdate();
			firstCall = TRUE;
		}
#endif
		if (Buttons__isPressed(&buttonRight))
		{
			if (colorCalibrationTimer - uC__getTaskIncrement() > 0)
			{
				colorCalibrationTimer = colorCalibrationTimer - uC__getTaskIncrement();
			}
			else
			{
				colorCalibrationTimer = 255;
				ESP_LOGI(LOG_TAG, "Switching to MODE__COLORCALIBRATION");
				Modes__setMode(MODE__COLORCALIBRATION, FALSE);
				LEDMatrix__enableUpdate();
				firstCall = TRUE;
			}
		}
		else
		{
			colorCalibrationTimer = 255;
		}

#if (DEBUG_MODE == DEBUG_MODE_ON)
		if (Buttons__isPressedOnce(&buttonLeft))
		{
			Modes__setMode(MODE__FAILUREMEMORY, FALSE);
			LEDMatrix__enableUpdate();
			firstCall = TRUE;
		}
#endif

#if (PROJECT == PROJECT__QLOCKTWO)
		if (Buttons__isPressed(&buttonMode))
		{
			if (langTimer - uC__getTaskIncrement() > 0)
			{
				langTimer = langTimer - uC__getTaskIncrement();
			}
			else
			{
				langTimer = 255;
				Qtwo__setNextLang();
				uC__triggerSwReset();
			}
		}
		else
		{
			langTimer = 255;
		}
#endif
	}
}
