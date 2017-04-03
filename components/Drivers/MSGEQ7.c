/*
 * MSGEQ7.c
 *
 *  Created on: 28.03.2017
 *      Author: Jean-Martin George
 */


#include "MSGEQ7.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#if (EQUALIZER == EQUALIZER_MSGEQ7)

void MSGEQ7__init (void)
{
	gpio_set_direction(MSGEQ7_RESET_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_direction(MSGEQ7_STROBE_GPIO, GPIO_MODE_OUTPUT);
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(MSGEQ7_ADC_CHANNEL, ADC_ATTEN_11db);

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

	for (it = 0; it < 7; it++)
	{
		gpio_set_level(MSGEQ7_STROBE_GPIO, 0);
		ets_delay_us(50);
		adcValues[it] = (uint16_t)adc1_get_voltage(MSGEQ7_ADC_CHANNEL);
		gpio_set_level(MSGEQ7_STROBE_GPIO, 1);
		ets_delay_us(50);
	}
}


void MSGEQ7__mainFunction (void *param)
{
	uint8_t it;
	uint16_t adcValues[7];
#if MSGEQ7DEBUG
	static uint8_t debugIt = 100;
#endif

	while (1)
	{
		MSGEQ7__readValues(&adcValues[0]);

#if MSGEQ7DEBUG

		if (debugIt > 0)
		{
			debugIt++;
		}
		else
		{
			printf("ADC values: ");

			for (it = 0; it < 7; it++)
			{
				printf("%d ", adcValues[it]);
			}

			printf("\n");
			debugIt = 100;
		}
#endif

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

#endif
