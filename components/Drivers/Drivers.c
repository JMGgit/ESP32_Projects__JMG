/*
 * Drivers.c
 *
 *  Created on: 22.02.2017
 *      Author: Jean-Martin George
 */


#include "Drivers.h"


void Drivers__init(void)
{
	MSGEQ7__init();
	APA102__init();
}
