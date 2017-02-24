/*
 * APA102.c
 *
 *  Created on: 22.02.2017
 *      Author: Jean-Martin George
 */


#include "APA102.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#if (LED_TYPE == LED_TYPE_APA102)
//#include "LEDMatrix.h"

#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
static uint8_t RGBLedOrder;
static uint8_t RGBLedOrder_EEPROM EEMEM;
#endif
static uint8_t updateEnabled = TRUE;
static uint8_t globalBrightness;

RGB_Color_t GS_Data[NUMBER_OF_LEDS];
/* - cannot be declared as static: ATMega limitation??
 * - defined as struct to save run time compared to simple buffer */

uint8_t APA102__getRGBLedOrder (void)
{
#if (RGB_LED_ORDER == RGB_LED_ORDER__RED_GREEN_BLUE)
	return RGB_LED_ORDER__RED_GREEN_BLUE;
#elif (RGB_LED_ORDER == RGB_LED_ORDER__BLUE_GREEN_RED)
	return RGB_LED_ORDER__BLUE_GREEN_RED;
#else
	return RGBLedOrder;
#endif
}


#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
void APA102__toggleRGBLedOrder (void)
{
	if (RGBLedOrder == RGB_LED_ORDER__RED_GREEN_BLUE)
	{
		RGBLedOrder = RGB_LED_ORDER__BLUE_GREEN_RED;
	}
	else
	{
		RGBLedOrder = RGB_LED_ORDER__RED_GREEN_BLUE;
	}

	eeprom_update_byte(&RGBLedOrder_EEPROM, RGBLedOrder);
}
#endif


spi_transaction_t spiTransaction;
spi_device_handle_t spi;

#define TX_BUFFER_LENGTH	(START_FRAME_LENGTH + (4 * NUMBER_OF_LEDS)+ STOP_FRAME_LENGTH)

uint8_t txBuffer[TX_BUFFER_LENGTH];

void APA102__updateAll (void)
{
    esp_err_t ret;
	memset(txBuffer, 0, sizeof(txBuffer));
	memset(&spiTransaction, 0, sizeof(spiTransaction));
	spiTransaction.tx_buffer = &txBuffer[0];
	spiTransaction.length = TX_BUFFER_LENGTH * 8;

	uint16_t it, bufferIdx;

	bufferIdx = 0;

	/* START FRAME*/
	for (it = 0; it < START_FRAME_LENGTH; it++)
	{
		txBuffer[bufferIdx++] = 0;
	}

	//bufferIdx = bufferIdx + START_FRAME_LENGTH;

	/* LED FRAMES */
	for (it = 0; it < NUMBER_OF_LEDS; it++)
	{
		txBuffer[bufferIdx++] = 0xE0 | globalBrightness;

		if (APA102__getRGBLedOrder() == RGB_LED_ORDER__BLUE_GREEN_RED)
		{
			txBuffer[bufferIdx++] = GS_Data[it].blue;
		}
		else
		{
			txBuffer[bufferIdx++] = GS_Data[it].red;
		}

		txBuffer[bufferIdx++] = GS_Data[it].green;

		if (APA102__getRGBLedOrder() == RGB_LED_ORDER__BLUE_GREEN_RED)
		{
			txBuffer[bufferIdx++] = GS_Data[it].red;
		}
		else
		{
			txBuffer[bufferIdx++] = GS_Data[it].blue;
		}
	}

	/* END FRAME */
	for (it = 0; it < STOP_FRAME_LENGTH; it++)
	{
		txBuffer[bufferIdx++] = 0xFF;
	}

    //ret=spi_device_transmit(spi, &spiTransaction);  //Transmit!
    ret=spi_device_queue_trans(spi, &spiTransaction, portMAX_DELAY);
    assert(ret==ESP_OK);

}


void APA102__x10 (void)
{
	if (updateEnabled)
	{
		APA102__updateAll();
	}
}


void APA102__setRGBForLED (RGB_Color_t color, uint16_t led)
{
	if (updateEnabled)
	{
		GS_Data[led] = color;
	}
}


void APA102__setRGBForAllLEDs (RGB_Color_t color)
{
	uint16_t idxLed;

	if (updateEnabled)
	{
		for (idxLed = 0; idxLed < NUMBER_OF_LEDS; idxLed++)
		{
			GS_Data[idxLed] = color;
		}
	}
}


void APA102__resetAllLEDs (void)
{
	if (updateEnabled)
	{
		memset(&GS_Data[0], 0, sizeof(GS_Data));
	}
}


void APA102__enableUpdate (uint8_t enable)
{
	updateEnabled = TRUE;
}


void APA102__disableUpdate (uint8_t enable)
{
	updateEnabled = FALSE;
}


void APA102__setGlobalBrightness (uint8_t brightness)
{
	if (brightness <= APA102_GLOBAL_BRIGHNESS__MAX)
	{
		globalBrightness = brightness;
	}
	else
	{
		globalBrightness = APA102_GLOBAL_BRIGHNESS__MAX;
	}
}


void APA102__init (void)
{
#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
	RGBLedOrder = eeprom_read_byte(&RGBLedOrder_EEPROM);

	if (	(RGBLedOrder != RGB_LED_ORDER__BLUE_GREEN_RED)
		&& 	(RGBLedOrder != RGB_LED_ORDER__RED_GREEN_BLUE)
		)
	{
		RGBLedOrder = RGB_LED_ORDER__RED_GREEN_BLUE;
		eeprom_update_byte(&RGBLedOrder_EEPROM, RGBLedOrder);
	}

#endif

    esp_err_t ret;

    spi_bus_config_t buscfg ={
        .miso_io_num=-1,
        .mosi_io_num=GPIO_NUM_23,
        .sclk_io_num=GPIO_NUM_18,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1
    };

    spi_device_interface_config_t devcfg={
        .clock_speed_hz=20000000,               //Clock out at 10 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=-1,               //CS pin
        .queue_size=5,                          //We want to be able to queue 7 transactions at a time
    };


    spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi);

    APA102__setGlobalBrightness(APA102_GLOBAL_BRIGHNESS__MAX);
}

#endif
