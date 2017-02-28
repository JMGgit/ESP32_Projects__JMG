/*
 * Stubs.h
 *
 *  Created on: 22.02.2017
 *      Author: Jean-Martin George
 */

#ifndef STUBS_H_
#define STUBS_H_

#include "stdint.h"

/* buttons */
#define BUTTONS_WIRED_OFF				1
#define BUTTONS_WIRED_PIN				2
#define BUTTONS_WIRED_HC165				3
#define BUTTONS_USART_OFF				1
#define BUTTONS_USART_ON				2
#define BUTTONS_IRMP_OFF				1
#define BUTTONS_IRMP_USED				2
#define BUTTONS_IRMP_SEND_TO_TWI		3
#define BUTTONS_TWI_OFF					1
#define BUTTONS_TWI_ON					2

#define BUTTONS_WIRED					BUTTONS_WIRED_OFF
#define BUTTONS_USART					BUTTONS_USART_OFF
#define BUTTONS_IRMP					BUTTONS_IRMP_OFF
#define BUTTONS_TWI						BUTTONS_TWI_OFF

#define LED_TYPE_APA102						1
#define LED_TYPE_WS2801						2
#define LED_TYPE_WS2812						3
#define LED_TYPE							LED_TYPE_APA102

#define RGB_LED_ORDER__RED_GREEN_BLUE		1
#define RGB_LED_ORDER__BLUE_GREEN_RED		2
#define RGB_LED_ORDER__CONFIGURABLE			3
#define RGB_LED_ORDER						RGB_LED_ORDER__BLUE_GREEN_RED

#define PROGMEM
#define EEMEM

static inline uint8_t eeprom_read_byte (uint8_t *x)
{
	return *x;
}


static inline void eeprom_update_byte (uint8_t *x, uint8_t y)
{
	*x = y;
}

static inline uint8_t pgm_read_byte (const uint8_t * x)
{
	return *x;
}


#define uC__getTaskIncrement()				(1)
#define uC__triggerSwReset()

#define LED_MATRIX_SIZE_LIN					LED_TABLE_SIZE_X
#define LED_MATRIX_SIZE_COL					LED_TABLE_SIZE_X

#define PROJECT__LED_TABLE					1
#define PROJECT								PROJECT__LED_TABLE

#define CLOCK_TYPE_ESP32					1
#define CLOCK_TYPE							CLOCK_TYPE_ESP32

#define CLOCK_SYNC_INTERNET					1
#define CLOCK_SYNC							CLOCK_SYNC_INTERNET

#define LED_ORDER__LEFT_2_RIGHT				1
#define LED_ORDER__STRAIGHT_FORWARD			2
#define LED_ORDER							LED_ORDER__LEFT_2_RIGHT

#define DEBUG_MODE_OFF						1
#define DEBUG_MODE							DEBUG_MODE_OFF

#define CLOCK_LED_DDR						1
#define CLOCK_LED_PIN						2
#define CLOCK_LED_PORT						3

#define SNAKE_BRIGHTNESS_LEVEL				80
#define SNAKE_SPEED							50

#define WHITE_COLOR_PERCENT_RED				80
#define WHITE_COLOR_PERCENT_GREEN			100
#define WHITE_COLOR_PERCENT_BLUE			100


#define setInput(ddr, pin)
#define isLow(port, pin)
#define isHigh(port, pin)

#define setOutput(ddr, pin)
#define setLow(port, pin)
#define setHigh(port, pin)

#define toggle(port, pin)

#endif /* STUBS_H_ */
