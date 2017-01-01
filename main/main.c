#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h>

#define UC_CTRL_LED_GPIO		GPIO_NUM_5

#define WIFI_SSID 				"Leo-Wohnung"
#define WIFI_PASSWORD 			"ObereBurghalde33"

#define UART1_TX_GPIO			GPIO_NUM_17
#define UART1_RX_GPIO			GPIO_NUM_16
#define UART1_RTS_GPIO			UART_PIN_NO_CHANGE
#define UART1_CTS_GPIO			UART_PIN_NO_CHANGE

/* Rx buffer not necessary but has to be greater UART_FIFO_LEN */
#define UART1_RX_BUFFER_LENGTH	(UART_FIFO_LEN + 1)

/* Tx buffer not needed for now */
#define UART1_TX_BUFFER_LENGTH	0

/* According to "Glediator protocol" */
#define UART_LED_FIRST_BYTE		1

#define LED_TEST_VALUE			100


esp_err_t event_handler(void *ctx, system_event_t *event)
{
	return ESP_OK;
}

#define NUMBER_OF_LEDS			1008
#define LED_TABLE_LENGTH		((3 * NUMBER_OF_LEDS) + 1)


uint8_t ledTable[LED_TABLE_LENGTH];


void app_main(void)
{
	nvs_flash_init();
	tcpip_adapter_init();

	esp_event_loop_init(event_handler, NULL);


	/********* WIFI CONFIG ************/

	wifi_config_t sta_config;
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_STA);

	memcpy(&(sta_config.sta.ssid), WIFI_SSID, sizeof(WIFI_SSID));
	memcpy(&(sta_config.sta.password), WIFI_PASSWORD, sizeof(WIFI_PASSWORD));

	sta_config.sta.bssid_set = false;

	esp_wifi_set_config(WIFI_IF_STA, &sta_config);
	esp_wifi_start();
	esp_wifi_connect();

	gpio_set_direction(UC_CTRL_LED_GPIO, GPIO_MODE_OUTPUT);


	/************ LED TABLE **************/

	ledTable[0] = UART_LED_FIRST_BYTE;

	for (uint16_t ledIt = 1; ledIt < LED_TABLE_LENGTH; ledIt++)
	{
		if ((ledIt - 1) % 3 == 2)
		{
			ledTable[ledIt] = LED_TEST_VALUE;
		}
	}


	/********** UART 1 CONFIG ***************/

	uart_config_t uartConfig;

	uartConfig.baud_rate = 1250000;
	uartConfig.data_bits = UART_DATA_8_BITS;
	uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
	uartConfig.parity = UART_PARITY_DISABLE;
	uartConfig.rx_flow_ctrl_thresh = 122;
	uartConfig.stop_bits = UART_STOP_BITS_1;

	uart_param_config(UART_NUM_1, &uartConfig);

	uart_set_pin(UART_NUM_1, UART1_TX_GPIO, UART1_RX_GPIO, UART1_RTS_GPIO, UART1_CTS_GPIO);

	/* no queue and interrupt for now */
	uart_driver_install(UART_NUM_1, UART1_RX_BUFFER_LENGTH, UART1_TX_BUFFER_LENGTH, 0, NULL, 0);


	/********** MAIN LOOP *************/

	uint8_t level = 0;

	while (1)
	{
		level = !level;
		gpio_set_level(UC_CTRL_LED_GPIO, level);
		uart_write_bytes(UART_NUM_1, (char*)&ledTable[0], LED_TABLE_LENGTH);

		/* add delay to avoid conflict with SPI output of ATMega (only one buffer is used for now) */
		vTaskDelay(5 / portTICK_PERIOD_MS);
	}
}

