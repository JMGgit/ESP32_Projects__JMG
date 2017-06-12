/*
 * uC.c
 *
 *  Created on: 18.02.2017
 *      Author: Jean-Martin George
 */

#include "uC.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Ota.h"


static uint8_t gpio_interrupt_num = 255;


void uC__triggerSwReset (void)
{
	esp_restart();
}


void uC__nvsInitStorage (const char *key, nvs_handle *nvsHandle)
{
	if (ESP_OK != nvs_open(key, NVS_READWRITE, nvsHandle))
	{
		printf("Error opening NVS key: %s\n", key);
	}
}


uint8_t uC__nvsReadByte (const char *key, nvs_handle nvsHandle, uint8_t *byte)
{
	if (ESP_OK != nvs_get_u8(nvsHandle, key, byte))
	{
		*byte = 0xFF;
		printf("Error reading NVS key: %s, handle: %d\n", key, nvsHandle);
	}

	printf("NVS key: %s, handle: %d, read value: %d\n", key, nvsHandle, *byte);

	return *byte;
}



void uC__nvsUpdateByte (const char *key, nvs_handle nvsHandle, uint8_t *byte_NVS, uint8_t byte)
{
	if (*byte_NVS != byte)
	{
		if (ESP_OK != nvs_set_u8(nvsHandle, key, byte))
		{
			printf("Error writing NVS key: %s, handle: %d, write value: %d\n", key, nvsHandle, byte);
		}

		if (ESP_OK != nvs_commit(nvsHandle))
		{
			printf("Error committing NVS key: %s, handle: %d, write value: %d\n", key, nvsHandle, byte);
		}

		/* assume storage was successful */
		*byte_NVS = byte;

		printf("NVS key: %s, handle: %d, write value: %d\n", key, nvsHandle, byte);
	}
}


void gpio__toggle (gpio_num_t gpio_num)
{
	static uint8_t gpioLevel = 0;

	if (!gpioLevel)
	{
		gpioLevel = 1;
	}
	else
	{
		gpioLevel = 0;
	}

	gpio_set_level(gpio_num, gpioLevel);
}


void gpio__handleInterrupt (void *para)
{
	gpio_interrupt_num = (uint32_t) para;
}


void uC__init (void)
{
	/* init non-volatile memory */
	nvs_flash_init();

	/* init LED for ESP32 thing board */
	gpio_pad_select_gpio(TEST_LED_BOARD_GPIO);
	gpio_set_direction(TEST_LED_BOARD_GPIO, GPIO_MODE_OUTPUT);

	/* init button for ESP32 thing board -> interrupt used for triggering SW reset and then SW update OTA */
	gpio_pad_select_gpio(BUTTON__BOARD_GPIO);
	gpio_set_direction(BUTTON__BOARD_GPIO, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON__BOARD_GPIO, GPIO_PULLUP_ONLY);
	gpio_set_intr_type(BUTTON__BOARD_GPIO, GPIO_INTR_POSEDGE);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(BUTTON__BOARD_GPIO, gpio__handleInterrupt, (void*) BUTTON__BOARD_GPIO);
}


void uC__mainFunction (void *param)
{
	while (1)
	{
		if (gpio_interrupt_num < 255)
		{
			printf("\nGPIO interrupt for GPIO: %d\n\n", gpio_interrupt_num);

			if (gpio_interrupt_num == BUTTON__BOARD_GPIO)
			{
				OTA__triggerSwUpdate();
			}

			gpio_interrupt_num = 255;
		}

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
