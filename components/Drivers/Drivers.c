/*
 * Drivers.c
 *
 *  Created on: 22.02.2017
 *      Author: Jean-Martin George
 */


#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#define LOG_TAG "DRIVERS"

#include "Drivers.h"
#include "esp_log.h"

void Drivers__init(void)
{
#if (EQUALIZER == EQUALIZER_MSGEQ7)
	MSGEQ7__init();
#endif
	APA102__init();

	ESP_LOGI(LOG_TAG, "Drivers__init done");
}
