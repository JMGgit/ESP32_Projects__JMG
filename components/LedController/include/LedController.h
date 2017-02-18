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
void LedController__storeLedData(uint8_t ledFrame, uint8_t *ledData, uint16_t ledDataLength, uint16_t ledDataStart);

#endif /* LEDCONTROLLER_H_ */
