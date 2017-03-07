#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "ArtNet.h"
#include "LedController.h"
#include "Wifi.h"
#include "Main_Config.h"
#include "Drivers.h"
#include <string.h>
#include "Modes.h"
#include "Clock.h"
#include "driver/timer.h"
#include "IRMP_Appl.h"
#include "Buttons.h"


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	Wifi__systemEvent(event);

	return ESP_OK;
}


void Main__init (void)
{
	nvs_flash_init();
	esp_event_loop_init(event_handler, NULL);
	
	gpio_set_direction(UC_CTRL_LED_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_direction(TEST_LED_ARTNET, GPIO_MODE_OUTPUT);
	gpio_set_direction(TEST_LED_LEDCTRL, GPIO_MODE_OUTPUT);
	
	Drivers__init();
	Wifi__init();
	Clock__init();
	LedController__init();
}



void LedTable__mainFunction (void *param)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 10 / portTICK_PERIOD_MS;

	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		if (!ArtNet__isActive())
		{
			gpio_set_level(TEST_LED_LEDCTRL, 1);
			//Modes__setMode(MODE__BLENDING_SWEEP_FAST, FALSE);
			Buttons__x10();
			Modes__x10();
			APA102__x10();
			gpio_set_level(TEST_LED_LEDCTRL, 0);
		}
	}
}


void Main__createTasks (void)
{
	/* Tasks for ArtNet controller */
#if 1
	if (pdPASS == xTaskCreate(LedTable__mainFunction, "LedTable__mainFunction", 1024, NULL, 1, NULL))
	{
		printf("Task LedTable__mainFunction created\n");
	}

#endif
#if 1
	if (pdPASS == xTaskCreate(ArtNet__mainFunction, "ArtNet__mainFunction", 1024, NULL, 1, NULL))
	{
		printf("Task ArtNet__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(ArtNet__debug, "ArtNet__debug", 1024, NULL, 10 , NULL))
	{
		printf("Task ArtNet__debug created\n");
	}

	if (pdPASS == xTaskCreate(LedController__mainFunction, "LedController__mainFunction", 1024, NULL, 1, NULL))
	{
		printf("Task LedController__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(Clock__mainFunction, "Clock__mainFunction", 1024, NULL, 1, NULL))
	{
		printf("Task Clock__mainFunction created\n");
	}
#endif
#if 0
	if (pdPASS ==  xTaskCreate(rmt_nec_rx_task, "rmt_nec_rx_task", 2048, NULL, 10, NULL))
	{
		printf("Task rmt_nec_rx_task created\n");
	}

	if (pdPASS ==  xTaskCreate(rmt_nec_tx_task, "rmt_nec_tx_task", 2048, NULL, 10, NULL))
	{
		printf("Task rmt_nec_tx_task created\n");
	}
#endif
}


IRMP_DATA irmp_data;

//------------------ User Task ---------------------

void irmpTask (void *param)
{
	while (1)
	{
		int rc = irmp_get_data (&irmp_data);

		if (rc)
		{
			printf("\nIRMP %10s(%2d): addr=0x%04x cmd=0x%04x, f=%d ",
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

#define TIMER_DIVIDER 1
#define TRK_TIMER_GROUP	TIMER_GROUP_0
#define TRK_TIMER_IDX	TIMER_1

void IRAM_ATTR timerIsr(void *para)
{
	gpio__toggle(TEST_LED_LEDCTRL);
    irmp_ISR();
	TIMERG0.int_clr_timers.t1 = 1;
	TIMERG0.hw_timer[TRK_TIMER_IDX].config.alarm_en = 1;
}


#define TIMER_INTERVAL (0.000066666)

void app_main (void)
{
	Main__init();
	Main__createTasks();
	irmp_init ();

	timer_config_t config;
	config.alarm_en = 1;
	config.auto_reload = 1;
	config.counter_dir = TIMER_COUNT_UP;
	config.divider = TIMER_DIVIDER;
	config.intr_type = TIMER_INTR_LEVEL;
	config.counter_en = TIMER_PAUSE;

	timer_init(TRK_TIMER_GROUP, TRK_TIMER_IDX, &config);
	timer_set_counter_value(TRK_TIMER_GROUP, TRK_TIMER_IDX, 0x00000000ULL);
	timer_enable_intr(TRK_TIMER_GROUP, TRK_TIMER_IDX);
	timer_isr_register(TRK_TIMER_GROUP, TRK_TIMER_IDX, timerIsr, NULL, ESP_INTR_FLAG_IRAM, NULL);

	timer_pause(TRK_TIMER_GROUP, TRK_TIMER_IDX);
	timer_set_counter_value(TRK_TIMER_GROUP, TRK_TIMER_IDX, 0x00000000ULL);
	//timer_set_alarm_value(TRK_TIMER_GROUP, TRK_TIMER_IDX, TIMER_INTERVAL * (TIMER_BASE_CLK / TIMER_DIVIDER));
	timer_set_alarm_value(TRK_TIMER_GROUP, TRK_TIMER_IDX, 2666);
	timer_start(TRK_TIMER_GROUP, TRK_TIMER_IDX);

	if (pdPASS ==  xTaskCreate(irmpTask, "irmpTask", 2048, NULL, 10, NULL))
	{
		printf("Task irmpTask created\n");
	}
}





