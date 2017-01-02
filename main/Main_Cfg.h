/*
 * Main_Cfg.h
 *
 *  Created on: 02.01.2017
 *      Author: Jean-Martin George
 */

#ifndef MAIN_CFG_H_
#define MAIN_CFG_H_


/********** CTL LED *********/
#define UC_CTRL_LED_GPIO		GPIO_NUM_5

/********* WIFI *********/
#define WIFI_SSID 				"Leo-Wohnung"
#define WIFI_PASSWORD 			"ObereBurghalde33"

/********* UART *********/
#define UART1_TX_GPIO			GPIO_NUM_17
#define UART1_RX_GPIO			GPIO_NUM_16
#define UART1_RTS_GPIO			UART_PIN_NO_CHANGE
#define UART1_CTS_GPIO			UART_PIN_NO_CHANGE
/* Rx buffer not necessary but has to be greater UART_FIFO_LEN */
#define UART1_RX_BUFFER_LENGTH	(UART_FIFO_LEN + 1)
/* Tx buffer not needed for now */
#define UART1_TX_BUFFER_LENGTH	0
/* According to "Glediator protocol" */
#define UART_LED_FIRST_BYTE		1

/********* LEDS *********/
#define NUMBER_OF_LEDS			1008
#define LED_TABLE_LENGTH		((3 * NUMBER_OF_LEDS) + 1)
#define LED_TEST_VALUE			100


#endif /* MAIN_CFG_H_ */
