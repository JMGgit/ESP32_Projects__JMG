/*
 * APA102.c
 *
 *  Created on: 22.02.2017
 *      Author: Jean-Martin George
 */


#include "APA102.h"
#include "esp_err.h"


#if (LED_TYPE == LED_TYPE_APA102)

/* SPI parameters */
spi_bus_config_t spiBusConfig;
spi_device_interface_config_t spiDeviceInterfaceConfig;
spi_transaction_t spiTransaction1, spiTransaction2;
spi_device_handle_t spiDeviceHandle;
#define SPI_TX_BUFFER1_LENGTH (START_FRAME_LENGTH + (4 * LEDS_NB))
#define SPI_TX_BUFFER2_LENGTH (STOP_FRAME_LENGTH)
uint8_t spiTxBuffer1[SPI_TX_BUFFER1_LENGTH];
uint8_t spiTxBuffer2[SPI_TX_BUFFER2_LENGTH];

#if ((SPI_TX_BUFFER1_LENGTH > 4095) || (SPI_TX_BUFFER2_LENGTH > 4095))
#error "SPI buffer to long! Reduce number of LEDs or update implementation of SPI buffers to avoid buffer (max size is 4095)"
#endif

#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
static uint8_t RGBLedOrder;
static nvs_handle nvsHandle_RGBLedOrder;
static uint8_t RGBLedOrder_NVS;
#endif
static uint8_t updateEnabled = TRUE;
static uint8_t globalBrightness;

RGB_Color_t GS_Data[LEDS_NB]; /* defined as struct to save run time compared to simple buffer */


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

	uC__nvsUpdateByte("RGBLedOrder", nvsHandle_RGBLedOrder, &RGBLedOrder_NVS, RGBLedOrder);
}
#endif



void APA102__updateAll (void)
{
	memset(&spiTransaction1, 0, sizeof(spiTransaction1));
	spiTransaction1.tx_buffer = &spiTxBuffer1[0];
	spiTransaction1.length = SPI_TX_BUFFER1_LENGTH * 8;

	uint16_t it, bufferIdx;

	bufferIdx = 0;

	/* START FRAME*/
	for (it = 0; it < START_FRAME_LENGTH; it++)
	{
		spiTxBuffer1[bufferIdx++] = 0;
	}

	/* LED FRAMES */
	for (it = 0; it < LEDS_NB; it++)
	{
		spiTxBuffer1[bufferIdx++] = 0xE0 | globalBrightness;

		if (APA102__getRGBLedOrder() == RGB_LED_ORDER__BLUE_GREEN_RED)
		{
			spiTxBuffer1[bufferIdx++] = GS_Data[it].blue;
		}
		else
		{
			spiTxBuffer1[bufferIdx++] = GS_Data[it].red;
		}

		spiTxBuffer1[bufferIdx++] = GS_Data[it].green;

		if (APA102__getRGBLedOrder() == RGB_LED_ORDER__BLUE_GREEN_RED)
		{
			spiTxBuffer1[bufferIdx++] = GS_Data[it].red;
		}
		else
		{
			spiTxBuffer1[bufferIdx++] = GS_Data[it].blue;
		}
	}

#if 0
	/* END FRAME */
	for (it = 0; it < STOP_FRAME_LENGTH; it++)
	{
		spiTxBuffer1[bufferIdx++] = 0xFF;
	}
#endif

	if (ESP_OK != spi_device_queue_trans(spiDeviceHandle, &spiTransaction1, portMAX_DELAY))
	{
		assert(0);
	}

#if 1
	spiTransaction2.tx_buffer = &spiTxBuffer2[0];
	spiTransaction2.length = STOP_FRAME_LENGTH * 8;
	bufferIdx = 0;


	/* END FRAME */
	for (it = 0; it < STOP_FRAME_LENGTH; it++)
	{
		spiTxBuffer2[bufferIdx++] = 0xFF;
	}

	if (ESP_OK != spi_device_queue_trans(spiDeviceHandle, &spiTransaction2, portMAX_DELAY))
	{
		assert(0);
	}
#endif
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
		for (idxLed = 0; idxLed < LEDS_NB; idxLed++)
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

	if (ESP_OK != spi_bus_initialize(HSPI_HOST, &spiBusConfig, 1))
	{
		assert(0);
	}

	if (ESP_OK != spi_bus_add_device(HSPI_HOST, &spiDeviceInterfaceConfig, &spiDeviceHandle))
	{
		assert(0);
	}

#if (RGB_LED_ORDER == RGB_LED_ORDER__CONFIGURABLE)
	uC__nvsInitStorage("RGBLedOrder", &nvsHandle_RGBLedOrder);

	RGBLedOrder = uC__nvsReadByte("RGBLedOrder", nvsHandle_RGBLedOrder, &RGBLedOrder_NVS);

	if (		(RGBLedOrder != RGB_LED_ORDER__BLUE_GREEN_RED)
			&& 	(RGBLedOrder != RGB_LED_ORDER__RED_GREEN_BLUE)
	)
	{
		RGBLedOrder = RGB_LED_ORDER__RED_GREEN_BLUE;
		uC__nvsUpdateByte("RGBLedOrder", nvsHandle_RGBLedOrder, &RGBLedOrder_NVS, RGBLedOrder);
	}

#endif

	APA102__setGlobalBrightness(APA102_GLOBAL_BRIGHNESS__MAX);
}

#endif
