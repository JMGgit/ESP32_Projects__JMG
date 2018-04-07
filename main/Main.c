#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_event_loop.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "ArtNet.h"
#include "LedController.h"
#include "Wifi.h"
#include "FOTA.h"
#include "Main_Config.h"
#include "Drivers.h"
#include <string.h>
#include "Modes.h"
#include "Clock.h"
#include "IRMP_Appl.h"
#include "Buttons.h"


#define STARTUP_TIME	250

TaskHandle_t taskHandle_LedTable__mainFunction;
TaskHandle_t taskHandle_ArtNet__mainFunction;
TaskHandle_t taskHandle_ArtNet__debug;
TaskHandle_t taskHandle_LedController__mainFunction;
TaskHandle_t taskHandle_Clock__mainFunction;
TaskHandle_t taskHandle_MSGEQ7__mainFunction;
TaskHandle_t taskHandle_uC__mainFunction;
TaskHandle_t taskHandle_FOTA__mainFunction;


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
	FOTA__init();
	Drivers__init();
	IRMP__init();
	Buttons__init();
	Clock__init();
	LEDMatrix__init();
	LedController__init();
	Modes__init();
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

		/* reset watchdog */
		esp_task_wdt_reset();
	}
}


void Main__createTasks (void)
{
	if (pdPASS == xTaskCreate(LedTable__mainFunction, "LedTable__mainFunction", 4096, NULL, 1, &taskHandle_LedTable__mainFunction))
	{
		esp_task_wdt_add(taskHandle_LedTable__mainFunction);
		printf("Task LedTable__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(ArtNet__mainFunction, "ArtNet__mainFunction", 4096, NULL, 1, &taskHandle_ArtNet__mainFunction))
	{
		esp_task_wdt_add(taskHandle_ArtNet__mainFunction);
		printf("Task ArtNet__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(ArtNet__debug, "ArtNet__debug", 4096, NULL, 10 , &taskHandle_ArtNet__debug))
	{
		esp_task_wdt_add(taskHandle_ArtNet__debug);
		printf("Task ArtNet__debug created\n");
	}

	if (pdPASS == xTaskCreate(LedController__mainFunction, "LedController__mainFunction", 4096, NULL, 1, &taskHandle_LedController__mainFunction))
	{
		esp_task_wdt_add(taskHandle_LedController__mainFunction);
		printf("Task LedController__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(Clock__mainFunction, "Clock__mainFunction", 4096, NULL, 2, &taskHandle_Clock__mainFunction))
	{
		esp_task_wdt_add(taskHandle_Clock__mainFunction);
		printf("Task Clock__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(MSGEQ7__mainFunction, "MSGEQ7__mainFunction", 4096, NULL, 3, &taskHandle_MSGEQ7__mainFunction))
	{
		esp_task_wdt_add(taskHandle_MSGEQ7__mainFunction);
		printf("Task MSGEQ7__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(uC__mainFunction, "uC__mainFunction", 4096, NULL, 1, &taskHandle_uC__mainFunction))
	{
		esp_task_wdt_add(taskHandle_uC__mainFunction);
		printf("Task uC__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(FOTA__mainFunction, "FOTA__mainFunction", 8192, NULL, 1, &taskHandle_FOTA__mainFunction))
	{
		/* no watchdog for FOTA */
		printf("Task FOTA__mainFunction created\n");
	}
}


void Main__deleteTask (TaskHandle_t taskHandle)
{
	esp_task_wdt_delete(taskHandle);
	vTaskDelete(taskHandle);
}


void app_main (void)
{
	Main__init();
	Main__createTasks();
}
