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
#include "driver/gpio.h"
#include "driver/uart.h"
#include "Main_Cfg.h"
#include "ArtNet.h"
#include "driver/timer.h"
#include <string.h>

typedef struct
{
	uint8_t frameNb;
	//uint8_t ledData[LED_TABLE_ARRAY_LENGTH - 1];
} ledData_t;

#define LED_BUFFER_SIZE 40

ledData_t ledDataTable[LED_BUFFER_SIZE];
uint8_t ledDataCounter;
uint8_t newFrameTrigger;
uint8_t lastFrame;
uint8_t ledBufferFull;

void LedController__storeLedData(uint8_t ledFrame, uint8_t *ledData, uint16_t ledDataLength, uint16_t ledDataStart)
{
	if (ledFrame != lastFrame)
	{
		ledDataTable[ledDataCounter].frameNb = ledFrame;
		//memcpy(&ledData[ledDataStart], ledData, ledDataLength);

		if (ledDataCounter < (LED_BUFFER_SIZE - 1))
		{
			ledDataCounter++;
		}
		else
		{
			ledDataCounter = 0;
			ledBufferFull = true;
		}

		newFrameTrigger = true;
	}

	lastFrame = ledFrame;
}

void LedController__mainFunction (void *param)
{
	while (1)
	{
		if (newFrameTrigger == true)
		{
			newFrameTrigger = false;

			if ((ledDataCounter % 40) == 0)
			{
				//printf("40 frames\n");
			}
		}

		if (ledBufferFull)
		{
//			printf("Led data stored for frames:");
//
//			for (uint8_t i = 0; i < LED_BUFFER_SIZE; i++)
//			{
//				printf(" %d", ledDataTable[i].frameNb);
//			}
//
//			printf("\n");
			ledBufferFull = false;
			memset(&ledDataTable, 0, sizeof(ledDataTable));
		}

		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}
