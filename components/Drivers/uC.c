/*
 * uC.c
 *
 *  Created on: 18.02.2017
 *      Author: Jean-Martin George
 */

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "UC"

#include "uC.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FOTA.h"
#include "esp_task_wdt.h"
#include "esp_log.h"


static uint8_t gpio_interrupt_num = 255;


void uC__triggerSwReset (void)
{
	esp_restart();
}


void uC__nvsInitStorage (const char *key, nvs_handle *nvsHandle)
{
	if (ESP_OK != nvs_open(key, NVS_READWRITE, nvsHandle))
	{
		ESP_LOGE(LOG_TAG, "Error opening NVS key: %s", key);
	}
}


uint8_t uC__nvsRead_u8 (const char *key, nvs_handle nvsHandle, uint8_t *value)
{
	if (ESP_OK != nvs_get_u8(nvsHandle, key, value))
	{
		*value = 0xFF;
		ESP_LOGE(LOG_TAG, "Error reading NVS key: %s, handle: %d", key, nvsHandle);
	}

	ESP_LOGI(LOG_TAG, "NVS key: %s, handle: %d, read value: %d", key, nvsHandle, *value);

	return *value;
}


uint64_t uC__nvsRead_u64 (const char *key, nvs_handle nvsHandle, uint64_t *value)
{
	if (ESP_OK != nvs_get_u64(nvsHandle, key, value))
	{
		*value = 0xFFFFFFFFFFFFFFFF;
		ESP_LOGE(LOG_TAG, "Error reading NVS key: %s, handle: %d", key, nvsHandle);
	}

	ESP_LOGI(LOG_TAG, "NVS key: %s, handle: %d, read value: %llu", key, nvsHandle, *value);

	return *value;
}


void uC__nvsUpdate_u8 (const char *key, nvs_handle nvsHandle, uint8_t *value_NVS, uint8_t value)
{
	if (*value_NVS != value)
	{
		if (ESP_OK != nvs_set_u8(nvsHandle, key, value))
		{
			ESP_LOGE(LOG_TAG, "Error writing NVS key: %s, handle: %d, write value: %d", key, nvsHandle, value);
		}

		if (ESP_OK != nvs_commit(nvsHandle))
		{
			ESP_LOGE(LOG_TAG, "Error committing NVS key: %s, handle: %d, write value: %d", key, nvsHandle, value);
		}

		/* assume storage was successful */
		*value_NVS = value;

		ESP_LOGI(LOG_TAG, "NVS key: %s, handle: %d, write value: %d", key, nvsHandle, value);
	}
}


void uC__nvsUpdate_u64 (const char *key, nvs_handle nvsHandle, uint64_t *value_NVS, uint64_t value)
{
	if (*value_NVS != value)
	{
		if (ESP_OK != nvs_set_u64(nvsHandle, key, value))
		{
			ESP_LOGE(LOG_TAG, "Error writing NVS key: %s, handle: %d, write value: %llu", key, nvsHandle, value);
		}

		if (ESP_OK != nvs_commit(nvsHandle))
		{
			ESP_LOGE(LOG_TAG, "Error committing NVS key: %s, handle: %d, write value: %llu", key, nvsHandle, value);
		}

		/* assume storage was successful */
		*value_NVS = value;

		ESP_LOGI(LOG_TAG, "NVS key: %s, handle: %d, write value: %llu", key, nvsHandle, value);
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
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        // OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);

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

	/* initialize watchdog */
	uc__enableWatchdog();

	ESP_LOGI(LOG_TAG, "uC__init done");
}


void uC__mainFunction (void *param)
{
	while (1)
	{
		if (gpio_interrupt_num < 255)
		{
			ESP_LOGI(LOG_TAG, "GPIO interrupt for GPIO: %d", gpio_interrupt_num);

			if (gpio_interrupt_num == BUTTON__BOARD_GPIO)
			{
				FOTA__triggerSwUpdate();
			}

			gpio_interrupt_num = 255;
		}

		vTaskDelay(100 / portTICK_PERIOD_MS);

		/* reset watchdog */
		esp_task_wdt_reset();
	}
}


void uc__enableWatchdog (void)
{
	ESP_LOGI(LOG_TAG, "Watchdog enabled");
	esp_task_wdt_init(WATCHDOG_PERIOD_SECONDS, TRUE);
}


void uc__disableWatchdog (void)
{
	ESP_LOGI(LOG_TAG, "Watchdog disabled");
	esp_task_wdt_deinit();
}
