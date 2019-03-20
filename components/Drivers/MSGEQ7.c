/*
 * MSGEQ7.c
 *
 *  Created on: 28.03.2017
 *      Author: Jean-Martin George
 */

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "MSGEQ7"

#include "MSGEQ7.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_log.h"

#if (EQUALIZER == EQUALIZER_MSGEQ7)

#define ADCVALUES_NB	7

void MSGEQ7__init (void)
{
	gpio_set_direction(MSGEQ7_RESET_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_direction(MSGEQ7_STROBE_GPIO, GPIO_MODE_OUTPUT);
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(MSGEQ7_ADC_CHANNEL, ADC_ATTEN_DB_11);

	/* reset peak */
	gpio_set_level(MSGEQ7_STROBE_GPIO, 1);
	gpio_set_level(MSGEQ7_RESET_GPIO, 0);
	ets_delay_us(1);
	gpio_set_level(MSGEQ7_RESET_GPIO, 1);
	ets_delay_us(1);
	gpio_set_level(MSGEQ7_RESET_GPIO, 0);
	ets_delay_us(100);
}


void MSGEQ7__readValues (uint16_t *adcValues)
{
	uint8_t it;

	for (it = 0; it < ADCVALUES_NB; it++)
	{
		gpio_set_level(MSGEQ7_STROBE_GPIO, 0);
		ets_delay_us(50);
		adcValues[it] = (uint16_t)adc1_get_raw(MSGEQ7_ADC_CHANNEL);
		gpio_set_level(MSGEQ7_STROBE_GPIO, 1);
		ets_delay_us(50);
	}
}


void MSGEQ7__readValueswithResolution (uint16_t *adcValues, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
	uint8_t it;

	MSGEQ7__readValues(adcValues);

	for (it = 0; it < ADCVALUES_NB; it++)
	{
		if (adcValues[it] < in_min)
		{
			adcValues[it] = out_min;
		}
		else if (adcValues[it] > in_max)
		{
			adcValues[it] = out_max;
		}
		else  if ((in_max - in_min) > (out_max - out_min))
		{
			adcValues[it] = (uint16_t)(adcValues[it] - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1) + out_min;
		}
		else
		{
			adcValues[it] = (uint16_t)(adcValues[it] - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		}
	}
}


void MSGEQ7__mainFunction (void *param)
{
#if MSGEQ7DEBUG
	uint16_t adcValues[ADCVALUES_NB];
	static uint8_t debugIt = 100;
	uint8_t it;
#endif

	while (1)
	{
#if MSGEQ7DEBUG

		MSGEQ7__readValueswithResolution(&adcValues[0]);

		if (debugIt > 0)
		{
			debugIt++;
		}
		else
		{
			ESP_LOGI(LOG_TAG, "ADC values: ");

			for (it = 0; it < ADCVALUES_NB; it++)
			{
				ESP_LOGI(LOG_TAG, "%d ", adcValues[it]);
			}

			ESP_LOGI(LOG_TAG, "\n");
			debugIt = 100;
		}
#endif

		vTaskDelay(10 / portTICK_PERIOD_MS);

		/* reset watchdog */
		esp_task_wdt_reset();
	}
}

#endif
