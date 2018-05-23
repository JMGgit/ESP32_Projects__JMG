/*
 * IRMP.c
 *
 *  Created on: 23.12.2014
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "IRMP"

#include "IRMP_Appl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "esp_log.h"

#if (BUTTONS_IRMP != BUTTONS_IRMP_OFF)

static void IRMP__testLed (uint8_t on)
{
	gpio_set_level(TEST_LED_IRMP_GPIO, on);
}


void IRAM_ATTR IRMP__interrupt(void *para)
{
	irmp_ISR();
	TIMERG0.int_clr_timers.t1 = 1;
	TIMERG0.hw_timer[TIMER_1].config.alarm_en = TIMER_ALARM_EN;
}


void IRMP__init (void)
{
	/* timer init */
	timer_config_t config;
	config.alarm_en = TIMER_ALARM_EN;
	config.auto_reload = TIMER_AUTORELOAD_EN;
	config.counter_dir = TIMER_COUNT_UP;
	config.divider = 2;
	config.intr_type = TIMER_INTR_LEVEL;
	config.counter_en = TIMER_PAUSE;
	timer_init(TIMER_GROUP_0, TIMER_1, &config);
	timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0x00000000ULL);
	timer_enable_intr(TIMER_GROUP_0, TIMER_1);
	timer_isr_register(TIMER_GROUP_0, TIMER_1, IRMP__interrupt, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, ((TIMER_BASE_CLK / config.divider) / F_INTERRUPTS));
	timer_start(TIMER_GROUP_0, TIMER_1);


	/* lib */
	irmp_init();

	/* callback to illuminate test LED */
	irmp_set_callback_ptr(&IRMP__testLed);

	ESP_LOGI(LOG_TAG, "IRMP__init done");
}


void IRMP__disable (void)
{
	timer_pause(TIMER_GROUP_0, TIMER_1);
	ESP_LOGI(LOG_TAG, "IRMP disabled");
}


void IRMP__enable (void)
{
	timer_start(TIMER_GROUP_0, TIMER_1);
	ESP_LOGI(LOG_TAG, "IRMP enabled");
}

uint8_t IRMP__readData (uint16_t address, uint8_t *data, uint8_t dataLength, uint8_t *repeat)
{
	uint8_t retVal = E_NOT_OK;
	IRMP_DATA IRMPData;

	if (dataLength > 2)
	{
		retVal = E_NOT_OK;
	}
	else
	{
		if (irmp_get_data(&IRMPData) == TRUE)
		{
			if (IRMPData.address == address)
			{
				if (dataLength == 1)
				{
					*data = (uint8_t)(IRMPData.command);
				}
				else
				{
					data[0] = (IRMPData.command) >> 8;
					data[1] = (IRMPData.command) & 0xFF;
				}

				if (IRMPData.flags & IRMP_FLAG_REPETITION)
				{
					*repeat = TRUE;
				}
			}

			retVal = E_OK;
		}
	}

	return retVal;
}


#if 0 /* only for debug! */
void IRMP__mainFunction (void *param)
{
	IRMP_DATA irmp_data;

	while (1)
	{
		if (irmp_get_data(&irmp_data) != 0)
		{
			ESP_LOGI(LOG_TAG, "\nIRMP %10s(%2d): addr=0x%04x cmd=0x%04x, f=%d ",
					irmp_protocol_names[ irmp_data.protocol],
					irmp_data.protocol,
					irmp_data.address,
					irmp_data.command,
					irmp_data.flags
			);
		}

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}
#endif


#endif
