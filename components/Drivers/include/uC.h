/*
 * uC.h
 *
 *  Created on: 18.02.2017
 *      Author: Jean-Martin George
 */

#ifndef UC_H_
#define UC_H_

#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "Main_Config.h"


void gpio__toggle (gpio_num_t gpio_num);
void uC__triggerSwReset (void);

void uC__nvsInitStorage (const char *key, nvs_handle *nvsHandle);
uint8_t uC__nvsReadByte (const char *key, nvs_handle nvsHandle, uint8_t *byte);
void uC__nvsUpdateByte (const char *key, nvs_handle nvsHandle, uint8_t *byte_NVS, uint8_t byte);
void uC__init (void);

#endif /* UC_H_ */
