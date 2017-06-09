#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "ArtNet.h"
#include "LedController.h"
#include "Wifi.h"
#include "OTA.h"
#include "Main_Config.h"
#include "Drivers.h"
#include <string.h>
#include "Modes.h"
#include "Clock.h"
#include "IRMP_Appl.h"
#include "Buttons.h"


#define STARTUP_TIME	250

static esp_err_t Main__eventHandler(void *ctx, system_event_t *event)
{
	Wifi__systemEvent(event);

	return ESP_OK;
}


void Main__init (void)
{
	esp_event_loop_init(Main__eventHandler, NULL);

	uC__init();
	Wifi__init();
	OTA__init();
	Drivers__init();
	IRMP__init();
	Buttons__init();
	Clock__init();
	LEDMatrix__init();
	LedController__init();
	Modes__init();
	APA102__x10();
}


void LedTable__mainFunction (void *param)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 10 / portTICK_PERIOD_MS;
	static uint32_t startupCounter = STARTUP_TIME;

	xLastWakeTime = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		if (startupCounter > 0)
		{
			startupCounter--;
		}
		else
		{
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
}


void Main__createTasks (void)
{
	if (pdPASS == xTaskCreate(LedTable__mainFunction, "LedTable__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task LedTable__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(ArtNet__mainFunction, "ArtNet__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task ArtNet__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(ArtNet__debug, "ArtNet__debug", 4096, NULL, 10 , NULL))
	{
		printf("Task ArtNet__debug created\n");
	}

	if (pdPASS == xTaskCreate(LedController__mainFunction, "LedController__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task LedController__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(Clock__mainFunction, "Clock__mainFunction", 4096, NULL, 2, NULL))
	{
		printf("Task Clock__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(MSGEQ7__mainFunction, "MSGEQ7__mainFunction", 4096, NULL, 3, NULL))
	{
		printf("Task MSGEQ7__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(uC__mainFunction, "uC__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task uC__mainFunction created\n");
	}
}


void app_main (void)
{
	Main__init();

	if (OTA__isSwUpdateTriggered())
	{
		OTA__enable();
	}
	else
	{
		Main__createTasks();
	}
}
