#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
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
#include "IRMP_Appl.h"
#include "Buttons.h"


static esp_err_t Main__eventHandler(void *ctx, system_event_t *event)
{
	Wifi__systemEvent(event);

	return ESP_OK;
}


void Main__init (void)
{
	nvs_flash_init();
	esp_event_loop_init(Main__eventHandler, NULL);

	gpio_set_direction(TEST_LED_BOARD_GPIO, GPIO_MODE_OUTPUT);

	Wifi__init();
	Drivers__init();
	IRMP__init();
	Buttons__init();
	Clock__init();
	LedController__init();
	Modes__init();
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
			gpio_set_level(TEST_LED_LEDCTRL_GPIO, 1);
			Buttons__x10();
			Modes__x10();
			APA102__x10();
			gpio_set_level(TEST_LED_LEDCTRL_GPIO, 0);
		}
	}
}


void Main__createTasks (void)
{
	/* Tasks for ArtNet controller */
	if (pdPASS == xTaskCreate(LedTable__mainFunction, "LedTable__mainFunction", 1024, NULL, 1, NULL))
	{
		printf("Task LedTable__mainFunction created\n");
	}

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

#if 0 /* only for debug */
	if (pdPASS ==  xTaskCreate(IRMP__mainFunction, "IRMP__mainFunction", 2048, NULL, 10, NULL))
	{
		printf("Task IRMP__mainFunction created\n");
	}
#endif
}


void app_main (void)
{
	Main__init();
	Main__createTasks();
}
