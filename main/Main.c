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
#include "Main_Cfg.h"
#include "Drivers.h"
#include <string.h>


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
	LedController__init();
}

void app_main_x10 (TimerHandle_t xTimer)
{
	static uint16_t artnetDebugCounter = 0;

	//gpio_set_level(TEST_LED_LEDCTRL, 1);
	//ArtNet__mainFunction(NULL);
	//LedController__mainFunction(NULL);

	if (artnetDebugCounter < 1000)
	{
		artnetDebugCounter++;
	}
	else
	{
		//ArtNet__debug(NULL);
		artnetDebugCounter = 0;
	}
	//gpio_set_level(TEST_LED_LEDCTRL, 0);
}


void Main__createTasks (void)
{
	TimerHandle_t timer_10ms;

	/* create a 10ms timer that will auto-reload */
	timer_10ms = xTimerCreate("timer_10ms", 10 / portTICK_PERIOD_MS, pdTRUE, (void*)0, app_main_x10);

	if ((timer_10ms != NULL) && (pdPASS == xTimerStart(timer_10ms, 10 )))
	{
		printf("Timer 10msTimer started\n");
	}


	/* Tasks for ArtNet controller */

	if (pdPASS == xTaskCreate(ArtNet__debug, "ArtNet__debug", 4096, NULL, 1 , NULL))
	{
		printf("Task ArtNet__debug created\n");
	}

	if ( pdPASS == xTaskCreate(LedController__mainFunction, "LedController__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task LedController__mainFunctionArtNet__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(ArtNet__mainFunction, "ArtNet__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task ArtNet__mainFunction created\n");
	}

}

void app_main (void)
{
	Main__init();
	Main__createTasks();

	while (1)
	{
		esp_task_wdt_feed();
	}
}

