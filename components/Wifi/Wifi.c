/*
 * Wifi.c
 *
 *  Created on: 08.02.2017
 *      Author: Jean-Martin George
 */


#include "Wifi.h"
#include "esp_wifi.h"
#include <string.h>
#include "Main_Config.h"
#include "Clock.h"


uint8_t wifiConnected;

uint8_t Wifi__isConnected (void)
{
	return wifiConnected;
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

	sta_config.sta.bssid_set = FALSE;

	esp_wifi_set_config(WIFI_IF_STA, &sta_config);
	esp_wifi_start();
	esp_wifi_connect();

	printf("Wifi__init done\n");
}


void Wifi__systemEvent (system_event_t *event)
{
	switch (event->event_id)
	{
		case SYSTEM_EVENT_STA_GOT_IP:
		{
			printf("Event handler: SYSTEM_EVENT_STA_GOT_IP -> wifiConnected = TRUE\n");
			wifiConnected = TRUE;
			break;
		}

		case SYSTEM_EVENT_STA_DISCONNECTED:
		{
			printf("Event handler: SYSTEM_EVENT_STA_DISCONNECTED -> wifiConnected = FALSE\n");
			wifiConnected = FALSE;
			Wifi__init();
			break;
		}

		default:
		{
			/* do nothing */
			break;
		}
	}
}
