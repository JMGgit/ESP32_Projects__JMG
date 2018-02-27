/*
 * LedTable_Main_Config.h
 *
 *  Created on: 10.04.2016
 *      Author: Jean-Martin George
 */

#ifndef DRIVERS_MAIN_CONFIG_H_
#define DRIVERS_MAIN_CONFIG_H_


#include "Drivers_Config_Def.h"
#include "Stubs.h"


#if (PROJECT == PROJECT__LED_TABLE)

/******* LEDs *******/
#define LED_TYPE					LED_TYPE_APA102
#define LEDS_NB						(LED_MATRIX_SIZE_COL * LED_MATRIX_SIZE_LIN)
#define LEDS_CHANNELS				(3 * LEDS_NB)
#if (LEDTABLE_REVISION == LEDTABLE_REVISION_1)
#define RGB_LED_ORDER				RGB_LED_ORDER__CONFIGURABLE
#else
#define RGB_LED_ORDER				RGB_LED_ORDER__CONFIGURABLE
#endif
#define LED_ORDER					LED_ORDER__CONFIGURABLE


/********** CTL LED *********/
#define TEST_LED_BOARD_GPIO			GPIO_NUM_5
#define TEST_LED_ARTNET_GPIO		GPIO_NUM_4
#define TEST_LED_LEDCTRL_GPIO		GPIO_NUM_25
#define TEST_LED_IRMP_GPIO			TEST_LED_BOARD_GPIO

/********** GPIO BUTTON *********/
#define BUTTON__BOARD_GPIO          GPIO_NUM_0

/******** MSGEQ7 **********/
#define MSGEQ7_RESET_GPIO			GPIO_NUM_19
#define MSGEQ7_STROBE_GPIO			GPIO_NUM_22
#define MSGEQ7_ADC_CHANNEL			ADC1_CHANNEL_0

/******* WATCHDOG *********/
#define WATCHDOG_PERIOD_SECONDS		20

#endif /* (PROJECT == PROJECT__LED_TABLE) */

#endif /* LEDTABLE_DRIVERS_CONFIG_H_ */
