/*
 * LedController.c
 *
 *  Created on: 08.02.2017
 *      Author: Jean-Martin George
 */


#include "LedController.h"
#include "freertos/FreeRTOS.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "Main_Config.h"
#include "ArtNet.h"
#include "driver/timer.h"
#include <string.h>
#include "Drivers.h"


uint8_t ledData[LEDS_CHANNELS];
uint8_t newDataTrigger;
static uint8_t updateEnabled = TRUE;


esp_err_t LedController__storeLedData(uint8_t *data, uint16_t start, uint16_t length)
{
	esp_err_t retVal;

	if (!newDataTrigger)
	{
		memcpy(&ledData[start], data, length);
		retVal = ESP_OK;
	}
	else
	{
		retVal = ESP_FAIL;
	}

	return retVal;
}


esp_err_t LedController__outputLedData (void)
{
	esp_err_t retVal = ESP_FAIL;

	if (!newDataTrigger && updateEnabled)
	{
		newDataTrigger = TRUE;
		retVal = ESP_OK;
	}

	return retVal;
}


void LedController__init (void)
{
    gpio_set_direction(TEST_LED_LEDCTRL_GPIO, GPIO_MODE_OUTPUT);
}


void LedController__mainFunction (void *param)
{
	uint16_t idxLed;

	while (1)
	{
		if (newDataTrigger)
		{
			gpio_set_level(TEST_LED_LEDCTRL_GPIO, 1);

#if (LED_TYPE == LED_TYPE_APA102)
			for (idxLed = 0; idxLed < LEDS_NB; idxLed++)
			{
				APA102__setRGBForLED(LEDMatrix__getRGBColorFromComponents(ledData[(3 * idxLed) + 2], ledData[(3 * idxLed) + 1], ledData[3 * idxLed]), idxLed);
			}

			APA102__x10();
#endif

			newDataTrigger = FALSE;

			gpio_set_level(TEST_LED_LEDCTRL_GPIO, 0);
		}

		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}


void LedController__enableUpdate (uint8_t enable)
{
	updateEnabled = TRUE;
}


void LedController__disableUpdate (uint8_t enable)
{
	updateEnabled = FALSE;
}
