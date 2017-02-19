/*
 * LedController.h
 *
 *  Created on: 08.02.2017
 *      Author: Jean-Martin George
 */

#ifndef LEDCONTROLLER_H_
#define LEDCONTROLLER_H_


#include <stddef.h>
#include "esp_err.h"


void LedController__mainFunction (void *param);

void LedController__storeLedData(uint8_t *data, uint16_t start, uint16_t length);
esp_err_t LedController__outputLedData (void);

#endif /* LEDCONTROLLER_H_ */
