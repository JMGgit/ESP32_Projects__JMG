#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h>


#define WIFI_SSID 		"ARRIS-55C2"
#define WIFI_PASSWORD 	"1Bonjour1"

#define UART_PORT		UART_NUM_1
#define UART_TX_GPIO	GPIO_NUM_1
#define UART_RX_GPIO	GPIO_NUM_1
#define UART_RTS_GPIO	GPIO_NUM_1
#define UART_CTS_GPIO	GPIO_NUM_1


esp_err_t event_handler(void *ctx, system_event_t *event)
{
	return ESP_OK;
}

#define NUMBER_OF_LEDS	1008

uint8_t ledTable[NUMBER_OF_LEDS + 1];


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

	strcpy(sta_config.sta.ssid, WIFI_SSID);
	strcpy(sta_config.sta.password, WIFI_PASSWORD);
	sta_config.sta.bssid_set = false;

	esp_wifi_set_config(WIFI_IF_STA, &sta_config);
	esp_wifi_start();
	esp_wifi_connect();

	gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);


	/************ LED TABLE **************/

	ledTable[0] = 1;

	for (uint16_t it = 1; it < (NUMBER_OF_LEDS + 1); it++)
	{
		if ((it - 1) % 3 == 0)
		{
			ledTable[it] = 100;
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

	uart_param_config(UART_PORT, &uartConfig);
	uart_set_pin(UART_PORT, UART_TX_GPIO, UART_RX_GPIO, UART_RTS_GPIO, UART_CTS_GPIO);



	/********** MAIN LOOP *************/

	uint8_t level = 0;

	while (true)
	{
		gpio_set_level(GPIO_NUM_5, level);
		level = !level;
		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}

