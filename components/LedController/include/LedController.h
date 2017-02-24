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
#include "Main_Cfg.h"


void LedController__init (void);
void LedController__mainFunction (void *param);

esp_err_t LedController__storeLedData(uint8_t *data, uint16_t start, uint16_t length);
esp_err_t LedController__outputLedData (void);

static inline  RGB_Color_t LEDMatrix__getRGBColorFromComponents (uint8_t red, uint8_t green, uint8_t blue)
{
	return ((RGB_Color_t){red, green, blue});
}

#endif /* LEDCONTROLLER_H_ */
