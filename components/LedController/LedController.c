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
#include "uC.h"
#include "driver/uart.h"
#include "Main_Cfg.h"
#include "ArtNet.h"
#include "driver/timer.h"
#include <string.h>

uint8_t ledData[LED_TABLE_ARRAY_LENGTH];
uint8_t newDataTrigger;


void LedController__storeLedData(uint8_t *data, uint16_t start, uint16_t length)
{
	if (!newDataTrigger)
	{
		memcpy(&ledData[1 + start], data, length);

		if ((1 + start + length) >  LED_TABLE_ARRAY_LENGTH)
		{
			printf("LedController: wrong data length!!\n");
		}
	}
}


esp_err_t LedController__outputLedData (void)
{
	esp_err_t retVal = ESP_FAIL;

	if (!newDataTrigger)
	{
		newDataTrigger = true;
		retVal = ESP_OK;
	}

	return retVal;
}


void LedController__init (void)
{
	ledData[0] = 1;
}


void LedController__mainFunction (void *param)
{
	while (1)
	{
		if (newDataTrigger)
		{
			//gpio__toggle(UART1_TX_GPIO);
			uart_write_bytes(UART_NUM_1, (char*)&ledData[0], LED_TABLE_ARRAY_LENGTH);
			newDataTrigger = false;
		}

		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}
