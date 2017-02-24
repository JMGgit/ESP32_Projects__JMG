#include "freertos/FreeRTOS.h"
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


void Main__createTasks (void)
{
	if (pdPASS == xTaskCreate(ArtNet__debug, "ArtNet__debug", 4096, NULL, 1 , NULL))
	{
		printf("Task LedController__mainFunction created\n");
	}

	if ( pdPASS == xTaskCreate(LedController__mainFunction, "LedController__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task ArtNet__mainFunction created\n");
	}

	if (pdPASS == xTaskCreate(ArtNet__mainFunction, "ArtNet__mainFunction", 4096, NULL, 1, NULL))
	{
		printf("Task ArtNet__debug created\n");
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

