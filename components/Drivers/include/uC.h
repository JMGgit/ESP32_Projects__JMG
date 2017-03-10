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


static inline void gpio__toggle (gpio_num_t gpio_num)
{
	static uint8_t gpioLevel = 0;

	if (!gpioLevel)
	{
		gpioLevel = 1;
	}
	else
	{
		gpioLevel = 0;
	}

	gpio_set_level(gpio_num, gpioLevel);
}

#endif /* UC_H_ */
