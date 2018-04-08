#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_event_loop.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "Main_Tasks.h"
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


#define STARTUP_TIME	200

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
				FOTA__enableCyclicCheckTemp();
				Buttons__x10();
				Modes__x10();
				APA102__x10();
				gpio_set_level(TEST_LED_LEDCTRL_GPIO, 0);
			}
			else
			{
				FOTA__disableCyclicCheckTemp();
			}
		}

		/* reset watchdog */
		esp_task_wdt_reset();
	}
}


void app_main (void)
{
	Main__init();
	Main__createAllTasks();
}
