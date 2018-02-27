/*
 * Drivers.c
 *
 *  Created on: 22.02.2017
 *      Author: Jean-Martin George
 */


#include "Drivers.h"


void Drivers__init(void)
{
#if (EQUALIZER == EQUALIZER_MSGEQ7)
	MSGEQ7__init();
#endif
	APA102__init();

	printf("Drivers__init done\n");
}
