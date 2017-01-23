#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "ArtNet.h"
#include "Main_Cfg.h"
#include <string.h>


uint8_t wifiConnected;
uint8_t artNetInitialized;
void Wifi__init (void);

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	switch (event->event_id)
	{
		case SYSTEM_EVENT_STA_GOT_IP:
		{
			printf("Event handler: SYSTEM_EVENT_STA_GOT_IP -> wifiConnected = true\n");
			wifiConnected = true;
			break;
		}

		case SYSTEM_EVENT_STA_DISCONNECTED:
		{
			printf("Event handler: SYSTEM_EVENT_STA_DISCONNECTED -> wifiConnected = false\n");
			wifiConnected = false;
			Wifi__init();
			break;
		}

		default:
		{
			/* do nothing */
			break;
		}
	}

	return ESP_OK;
}


void Wifi__init (void)
{
	wifi_config_t sta_config;
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	tcpip_adapter_init();
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_STA);

	memcpy(&(sta_config.sta.ssid), WIFI_SSID, sizeof(WIFI_SSID));
	memcpy(&(sta_config.sta.password), WIFI_PASSWORD, sizeof(WIFI_PASSWORD));

	sta_config.sta.bssid_set = false;

	esp_wifi_set_config(WIFI_IF_STA, &sta_config);
	esp_wifi_start();
	esp_wifi_connect();

	printf("Wifi__init done\n");
}


void UART1__init (void)
{
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

	printf("UART1__init done\n");
}


void Main__init (void)
{
	nvs_flash_init();
	esp_event_loop_init(event_handler, NULL);
	gpio_set_direction(UC_CTRL_LED_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_direction(UC_TEST1_GPIO, GPIO_MODE_OUTPUT);

	Wifi__init();
	UART1__init();

	printf("Main__init done\n\n");
}


void app_main (void)
{
	Main__init();

	while (1)
	{
		if (wifiConnected)
		{
			if (!artNetInitialized)
			{
				if (ArtNet__init() == ESP_OK)
				{
					artNetInitialized = true;
				}
			}
			else
			{
				ArtNet__mainFunction();
			}
		}

		esp_task_wdt_feed();
	}
}

