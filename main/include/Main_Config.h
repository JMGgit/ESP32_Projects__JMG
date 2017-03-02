/*
 * Main_Config.h
 *
 *  Created on: 02.01.2017
 *      Author: Jean-Martin George
 */

#ifndef MAIN_CONFIGH_
#define MAIN_CONFIGH_

#include "Wifi_SSID_Cfg.h"
#include "Stubs.h"


/********** CTL LED *********/
#define UC_CTRL_LED_GPIO				GPIO_NUM_5
#define TEST_LED_ARTNET					GPIO_NUM_4
#define TEST_LED_LEDCTRL				GPIO_NUM_25


/********* LEDS *********/
#define LED_TABLE_SIZE_LIN				24
#define LED_TABLE_SIZE_COL				42
#define NUMBER_OF_LEDS					(LED_TABLE_SIZE_LIN * LED_TABLE_SIZE_COL)
#define NUMBER_OF_LEDS_CHANNELS			(3* NUMBER_OF_LEDS)


#endif /* MAIN_CONFIGH_ */
