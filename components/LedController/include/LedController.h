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
#include "Main_Types.h"
#include "Main_Config.h"
#include "LedMatrix.h"


void LedController__init (void);
void LedController__mainFunction (void *param);

esp_err_t LedController__storeLedData(uint8_t *data, uint16_t start, uint16_t length);
esp_err_t LedController__outputLedData (void);

void LedController__enableUpdate (uint8_t enable);
void LedController__disableUpdate (uint8_t enable);


#endif /* LEDCONTROLLER_H_ */
