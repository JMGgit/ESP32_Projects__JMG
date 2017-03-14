/*
 * uC.c
 *
 *  Created on: 18.02.2017
 *      Author: Jean-Martin George
 */

#include "uC.h"


void uC__triggerSwReset (void)
{
	esp_restart();
}


uint8_t eeprom_read_byte (uint8_t *x)
{
	return *x;
}


void eeprom_update_byte (uint8_t *x, uint8_t y)
{
	*x = y;
}
