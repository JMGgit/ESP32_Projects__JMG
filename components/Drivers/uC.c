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
