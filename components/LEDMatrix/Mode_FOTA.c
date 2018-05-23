/*
 * Moe_FOTA.c
 *
 *  Created on: 07.04.2018
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "MODE_OFF"


#include "Mode_FOTA.h"
#include "Modes.h"
#include "esp_log.h"


#if (FOTA_SW_UPDATE == FOTA_SW_UPDATE_ON)

void ModeFOTA__x10 (void)
{
	FOTA_State_t fotaState;

	fotaState = FOTA__getCurrentState();

	if (Buttons__isPressedOnce(&buttonUp))
	{
		FOTA__triggerSwUpdate();
	}

	switch (fotaState)
	{
	case FOTA_STATE_NO_UPDATE:
	{
		LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(10, 10 , 10));
		break;
	}

	case FOTA_STATE_ERROR:
	{
		LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(100, 0 , 0));
		break;
	}

	case FOTA_STATE_CONNECTION_IN_PROGRESS:
	{
		LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(0, 0 , 100));
		break;
	}

	case FOTA_STATE_UPDATE_IN_PROGRESS:
	{
		LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(100, 65 , 0));
		break;
	}

	case FOTA_STATE_UPDADE_FINISHED:
	{
		LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(0, 100 , 0));
		break;
	}

	default:
	{
		break;
	}
	}


	/** EXIT conditions **/

	if (Buttons__isPressedOnce(&buttonOff))
	{
		if (fotaState == FOTA_STATE_UPDADE_FINISHED)
		{
			uC__triggerSwReset();
		}
		else
		{
			FOTA__disableCyclicCheckTemp();
			Modes__Start();
		}
	}
}

#endif /* (FOTA_SW_UPDATE == FOTA_SW_UPDATE_ON)*/
