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
#include <string.h>


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	Wifi__systemEvent(event);

	return ESP_OK;
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

	/* test */
	//gpio_set_direction(UART1_TX_GPIO, GPIO_MODE_OUTPUT);

	Wifi__init();
	UART1__init();

	printf("Main__init done\n");
}


void Main__createTasks (void)
{
	xTaskCreate(LedController__mainFunction, "LedController__mainFunction", 4096, NULL, 5, NULL);
	xTaskCreate(ArtNet__mainFunction, "ArtNet__mainFunction", 4096, NULL, 5, NULL);
	printf("Tasks created\n");
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

