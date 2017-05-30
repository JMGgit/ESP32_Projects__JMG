/*
 * Mode_Off.c
 *
 *  Created on: 07.04.2016
 *      Author: Jean-Martin George
 */


#include "Mode_Off.h"
#include "Modes.h"
#if (PROJECT == PROJECT__QLOCKTWO)
#include "Modes_Qlocktwo/Modes_Qlocktwo.h"
#endif

static uint8_t firstCall = TRUE;

void Off__x10 (void)
{
	static uint8_t rgbConnectionTimer = 255;
	static uint8_t ledConnectionTimer = 255;
	static uint8_t startupTimer = 255;
#if (PROJECT == PROJECT__QLOCKTWO)
	static uint8_t langTimer = 255;
#endif

	if (firstCall)
	{
		LEDMatrix__clearMatrix();
#if (OTA_SW_UPDATE == OTA_SW_UPDATE_ON)
		OTA__enable();
#endif
		firstCall = FALSE;
	}
	else
	{
		LEDMatrix__disableUpdate();

		if (Buttons__isPressedOnce(&buttonOff))
		{
#if (OTA_SW_UPDATE == OTA_SW_UPDATE_ON)
			OTA__disable();
#endif
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

#if (OTA_SW_UPDATE == OTA_SW_UPDATE_ON)
		if (OTA__isUpdateInProgress())
		{
			LEDMatrix__enableUpdate();
			LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(100, 0 , 0));
		}
		else if (OTA__isNewSwFlashed())
		{
			LEDMatrix__enableUpdate();
			LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(0, 100 , 0));
		}
		else
		{
			/* nothing to do */
		}
#endif

		if (Buttons__isPressedOnce(&buttonDown))
		{
			uC__triggerSwReset();
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
