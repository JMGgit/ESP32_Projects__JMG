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

/* SPI parameters */
spi_bus_config_t spiBusConfig;
spi_device_interface_config_t spiDeviceInterfaceConfig;
spi_transaction_t spiTransaction;
spi_device_handle_t spiDeviceHandle;
#define SPI_TX_BUFFER_LENGTH (START_FRAME_LENGTH + (4 * NUMBER_OF_LEDS)+ STOP_FRAME_LENGTH)
uint8_t spiTxBuffer[SPI_TX_BUFFER_LENGTH];


#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
static uint8_t RGBLedOrder;
static uint8_t RGBLedOrder_EEPROM EEMEM;
#endif
static uint8_t updateEnabled = TRUE;
static uint8_t globalBrightness;

RGB_Color_t GS_Data[NUMBER_OF_LEDS]; /* defined as struct to save run time compared to simple buffer */

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



void APA102__updateAll (void)
{
	memset(&spiTransaction, 0, sizeof(spiTransaction));
	spiTransaction.tx_buffer = &spiTxBuffer[0];
	spiTransaction.length = SPI_TX_BUFFER_LENGTH * 8;

	uint16_t it, bufferIdx;

	bufferIdx = 0;

	/* START FRAME*/
	for (it = 0; it < START_FRAME_LENGTH; it++)
	{
		spiTxBuffer[bufferIdx++] = 0;
	}

	/* LED FRAMES */
	for (it = 0; it < NUMBER_OF_LEDS; it++)
	{
		spiTxBuffer[bufferIdx++] = 0xE0 | globalBrightness;

		if (APA102__getRGBLedOrder() == RGB_LED_ORDER__BLUE_GREEN_RED)
		{
			spiTxBuffer[bufferIdx++] = GS_Data[it].blue;
		}
		else
		{
			spiTxBuffer[bufferIdx++] = GS_Data[it].red;
		}

		spiTxBuffer[bufferIdx++] = GS_Data[it].green;

		if (APA102__getRGBLedOrder() == RGB_LED_ORDER__BLUE_GREEN_RED)
		{
			spiTxBuffer[bufferIdx++] = GS_Data[it].red;
		}
		else
		{
			spiTxBuffer[bufferIdx++] = GS_Data[it].blue;
		}
	}

	/* END FRAME */
	for (it = 0; it < STOP_FRAME_LENGTH; it++)
	{
		spiTxBuffer[bufferIdx++] = 0xFF;
	}

	if (ESP_OK != spi_device_queue_trans(spiDeviceHandle, &spiTransaction, portMAX_DELAY))
	{
		assert(0);
	}
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
	/* initialize SPI controller */

	spiBusConfig.miso_io_num = -1;
	spiBusConfig.mosi_io_num = GPIO_NUM_23;
	spiBusConfig.sclk_io_num = GPIO_NUM_18;
	spiBusConfig.quadwp_io_num = -1;
	spiBusConfig.quadhd_io_num = -1;

	spiDeviceInterfaceConfig.clock_speed_hz = 20000000;
	spiDeviceInterfaceConfig.mode = 0;
	spiDeviceInterfaceConfig.spics_io_num = -1;
	spiDeviceInterfaceConfig.queue_size = 5;

	if (E_OK != spi_bus_initialize(HSPI_HOST, &spiBusConfig, 1))
	{
		assert(0);
	}

	if (E_OK != spi_bus_add_device(HSPI_HOST, &spiDeviceInterfaceConfig, &spiDeviceHandle))
	{
		assert(0);
	}

#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
	RGBLedOrder = eeprom_read_byte(&RGBLedOrder_EEPROM);

	if (		(RGBLedOrder != RGB_LED_ORDER__BLUE_GREEN_RED)
			&& 	(RGBLedOrder != RGB_LED_ORDER__RED_GREEN_BLUE)
	)
	{
		RGBLedOrder = RGB_LED_ORDER__RED_GREEN_BLUE;
		eeprom_update_byte(&RGBLedOrder_EEPROM, RGBLedOrder);
	}

#endif

	APA102__setGlobalBrightness(APA102_GLOBAL_BRIGHNESS__MAX);
}

#endif
