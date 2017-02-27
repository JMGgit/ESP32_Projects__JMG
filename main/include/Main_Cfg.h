/*
 * Main_Cfg.h
 *
 *  Created on: 02.01.2017
 *      Author: Jean-Martin George
 */

#ifndef MAIN_CFG_H_
#define MAIN_CFG_H_

#include "Wifi_SSID_Cfg.h"


/********** CTL LED *********/
#define UC_CTRL_LED_GPIO				GPIO_NUM_5
#define TEST_LED_ARTNET					GPIO_NUM_4
#define TEST_LED_LEDCTRL				GPIO_NUM_25


/********* LEDS *********/
#define LED_TABLE_SIZE_X				42
#define LED_TABLE_SIZE_Y				24
#define NUMBER_OF_LEDS					(LED_TABLE_SIZE_X * LED_TABLE_SIZE_Y)
#define NUMBER_OF_LEDS_CHANNELS			(3* NUMBER_OF_LEDS)


#endif /* MAIN_CFG_H_ */
