/*
 * Modes.c
 *
 *  Created on: 20.01.2013
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "MODES"

#include "Modes_LedTable.h"
#include "esp_log.h"


static Mode_t currentMode;
static uint16_t timerModeChange;
uint8_t mode_NVS;
static nvs_handle nvsHandle_mode;
static uint8_t startupOn;
static uint8_t startupOn_NVS;
static nvs_handle nvsHandle_startupOn;


uint16_t timerModeChangeConf[MODE_NB] =
{
		0xFFFF,	/* MODE__STARTUP = 0 */
		0xFFFF,	/* MODE__FAILUREMEMORY */
		0xFFFF,	/* MODE__FOTA */
		0xFFFF,	/* MODE__OFF */
		0xFFFF,	/* MODE__ALL_ON */
		0xFFFF,	/* MODE__BLENDING_SLOW */
		0xFFFF, /* MODE__BLENDING_SLOW_2_COLORS */
		0xFFFF,	/* MODE__BLENDING_FAST */
		0xFFFF,	/* MODE__BLENDING_FAST_2_COLORS */
		0xFFFF,	/* MODE__BLENDING_SWEEP */
		0xFFFF,	/* MODE__BLENDING_SWEEP_FAST */
		0xFFFF,	/* MODE__BLENDING_CLOCK */
		0xFFFF,	/* MODE__BLENDING_CLOCK_INVERTED */
		0xFFFF,	/* MODE__BLENDING_CLOCK_FAST */
		0xFFFF,	/* MODE__BLENDING_CLOCK_INVERTED_FAST */
		0xFFFF,	/* MODE__CLOCK */
		0xFFFF,	/* MODE__SNAKE */
		0xFFFF	/* MODE__EQUALIZER */
};


 void Modes__transition (void)
{
	if (currentMode == MODE__SNAKE)
	{
		Snake__init();
	}

	if (currentMode == MODE__EQUALIZER)
	{
		Equalizer__init();
	}
}


 void Modes__setMode (Mode_t mode, uint8_t transition)
{
	if (mode < MODE_NB)
	{
		currentMode = mode;
	}
	else
	{
		currentMode = MODE__INIT;
	}

	if (transition)
	{
		Modes__transition();
	}
}


void Modes__Start (void)
{
#if (DEBUG_MODE == DEBUG_MODE_ON)
	if (FailureMemory__getFaultCounter() > 0)
	{
		Modes__setMode(MODE__FAILUREMEMORY, FALSE);
	}
	else
#endif
	{
		if (uC__nvsRead_u8("mode", nvsHandle_mode, &mode_NVS) < MODE__INIT)
		{
			Modes__setMode(MODE__INIT, TRUE);
		}
		else
		{
			Modes__setMode(uC__nvsRead_u8("mode", nvsHandle_mode, &mode_NVS), TRUE);
		}
	}
}


static void Modes__setNextMode (void)
{
	Modes__setMode(currentMode + 1, TRUE);
}


Mode_t Modes__getMode (void)
{
	return currentMode;
}


static void Mode__eepromStorage (void)
{
	uC__nvsUpdate_u8("mode", nvsHandle_mode, &mode_NVS, currentMode);
}


static void Modes__updateMatrix (void)
{
	switch (currentMode)
	{
		case MODE__STARTUP:
		{
			Startup__x10();
			break;
		}

		case MODE__FAILUREMEMORY:
		{
			FailureMemory__x10();
			break;
		}

		case MODE__OFF:
		{
			Off__x10();
			break;
		}

		case MODE__ALL_ON:
		{
			AllOn__x10();
			break;
		}

		case MODE__BLENDING_SLOW:
		{
			ColorBlending__x10(BLENDING_MODE_SLOW);
			break;
		}

		case MODE__BLENDING_SLOW_2_COLORS:
		{
			ColorBlending__x10(BLENDING_MODE_SLOW_2_COLORS);
			break;
		}

		case MODE__BLENDING_FAST:
		{
			ColorBlending__x10(BLENDING_MODE_FAST);
			break;
		}

		case MODE__BLENDING_FAST_2_COLORS:
		{
			ColorBlending__x10(BLENDING_MODE_FAST_2_COLORS);
			break;
		}

		case MODE__BLENDING_SWEEP:
		{
			ColorBlending__x10(BLENDING_MODE_SWEEP);
			break;
		}

		case MODE__BLENDING_SWEEP_FAST:
		{
			ColorBlending__x10(BLENDING_MODE_SWEEP_FAST);
			break;
		}

		case MODE__BLENDING_CLOCK:
		{
			ModeClock__x10(CLOCK_MODE_COLOR_BLENDING);
			break;
		}

		case MODE__BLENDING_CLOCK_INVERTED:
		{
			ModeClock__x10(CLOCK_MODE_COLOR_BLENDING_INVERTED);
			break;
		}

		case MODE__BLENDING_CLOCK_FAST:
		{
			ModeClock__x10(CLOCK_MODE_COLOR_BLENDING_FAST);
			break;
		}

		case MODE__BLENDING_CLOCK_INVERTED_FAST:
		{
			ModeClock__x10(CLOCK_MODE_COLOR_BLENDING_INVERTED_FAST);
			break;
		}

		case MODE__CLOCK:
		{
			ModeClock__x10(CLOCK_MODE_ONE_COLOR);
			Modes__setNextMode();
			break;
		}

		case MODE__SNAKE:
		{
			Snake__x10(SNAKE_BRIGHTNESS_LEVEL);
			break;
		}

		case MODE__EQUALIZER:
		{
			Equalizer__x10();
			break;
		}

		case MODE__FOTA:
		{
			ModeFOTA__x10();
			break;
		}

		default:
		{
			break;
		}
	}

	if (currentMode >= MODE__INIT)
	{
		Mode__eepromStorage();
	}

	if ((FOTA__getCurrentState() == FOTA_STATE_ERROR) || (FOTA__getCurrentState() == FOTA_STATE_UPDADE_FINISHED))
	{
		/* display last FOTA result on led matrix */
		Modes__setMode(MODE__FOTA, FALSE);
	}
}


void Modes__init (void)
{
	LEDMatrix__clearMatrix();

	uC__nvsInitStorage("mode", &nvsHandle_mode);
	uC__nvsInitStorage("startupOn", &nvsHandle_startupOn);

	if (uC__nvsRead_u8("startupOn", nvsHandle_startupOn, &startupOn_NVS) == TRUE)
	{
		Modes__setMode(MODE__STARTUP, FALSE);
		startupOn = TRUE;
	}
	else
	{
		Modes__Start();
		startupOn = FALSE;
		uC__nvsUpdate_u8("startupOn", nvsHandle_startupOn, &startupOn_NVS, startupOn);
	}

	ModeClock__init();

    ESP_LOGI(LOG_TAG, "Modes__init done");
}


void Modes__x10 (void)
{
	if ((currentMode != MODE__OFF) && (currentMode != MODE__STARTUP))
	{
		if (Buttons__isPressedOnce(&buttonMode))
		{
			if (currentMode != MODE__FAILUREMEMORY)
			{
				Modes__setNextMode();
				timerModeChange = 0;
			}
			else
			{
				Modes__Start();
			}
		}
		else if (timerModeChangeConf[currentMode] != 0xFFFF)
		{
			if (timerModeChange + uC__getTaskIncrement() < timerModeChangeConf[currentMode])
			{
				timerModeChange = timerModeChange + uC__getTaskIncrement();
			}
			else
			{
				Modes__setNextMode();
				timerModeChange = 0;
			}
		}
		else
		{
			/* nothing to do */
		}

		if ((Buttons__isPressedOnce(&buttonOff)) && (currentMode >= MODE__INIT))
		{
			Modes__setMode(MODE__OFF, FALSE);
		}
	}

	Modes__updateMatrix();
}


void Modes__toggleStartupMode (void)
{
	if (startupOn)
	{
		startupOn = FALSE;
	}
	else
	{
		startupOn = TRUE;
	}

	uC__nvsUpdate_u8("startupOn", nvsHandle_startupOn, &startupOn_NVS, startupOn);
}
