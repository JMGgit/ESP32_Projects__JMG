/*
 * uC.c
 *
 *  Created on: 18.02.2017
 *      Author: Jean-Martin George
 */

#include "uC.h"


void uC__triggerSwReset (void)
{
	esp_restart();
}


void uC__nvsInitStorage (const char *key, nvs_handle *nvsHandle)
{
	if (ESP_OK != nvs_open(key, NVS_READWRITE, nvsHandle))
	{
		printf("Error opening NVS!\n");
	}
}


uint8_t uC__nvsReadByte (const char *key, nvs_handle nvsHandle, uint8_t *byte)
{
	if (ESP_OK != nvs_get_u8(nvsHandle, key, byte))
	{
		*byte = 0xFF;
		printf("Error reading testCounter!\n");
	}

	printf("NVS item: %d, read value: %d\n", nvsHandle, *byte);

	return *byte;
}



void uC__nvsUpdateByte (const char *key, nvs_handle nvsHandle, uint8_t *byte_NVS, uint8_t byte)
{
	if (*byte_NVS != byte)
	{
		if (ESP_OK != nvs_set_u8(nvsHandle, key, byte))
		{
			printf("Error writing testCounter!\n");
		}

		if (ESP_OK != nvs_commit(nvsHandle))
		{
			printf("Error committing testCounter!\n");
		}

		/* assume storage was successful */
		*byte_NVS = byte;

		printf("NVS item: %d, write value: %d\n", nvsHandle, byte);
	}
}

void uC__init (void)
{
	nvs_flash_init();
	gpio_set_direction(TEST_LED_BOARD_GPIO, GPIO_MODE_OUTPUT);
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
