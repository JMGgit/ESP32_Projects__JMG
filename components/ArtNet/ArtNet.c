/*
 * ArtNet.c
 *
 *  Created on: 02.01.2017
 *      Author: Jean-Martin George
 */

#include "Main_Cfg.h"
#include "ArtNet.h"
#include "lwip/udp.h"
#include "lwip/debug.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h>

uint8_t udpData[ARTNET_MAX_DATA_LENGTH];
uint8_t ledTable[LED_TABLE_ARRAY_LENGTH];


esp_err_t ArtNet__DecodeDmxFrame (const uint8_t *buffer, uint8_t **ledDataPtr, uint16_t *dataLength)
{
	esp_err_t retVal = ESP_FAIL;

	const uint16_t opCode = (buffer[9] << 8) + buffer[8];
#if ARTNET_DEBUG_FRAME_INFO
	const uint16_t protVersion = (buffer[10] << 8) + buffer[11];
	const uint8_t seqNum = buffer[12];
	const uint8_t physInput = buffer[13];
#endif
	const uint8_t subNet = buffer[14] / 16;
	const uint8_t universe = buffer[14] % 16;
	const uint8_t net = buffer[15];
	const uint16_t length =(buffer[16] << 8) + buffer[17];

	if (	buffer[0] == (uint8_t)'A'
		&&	buffer[1] == (uint8_t)'r'
		&&	buffer[2] == (uint8_t)'t'
		&&	buffer[3] == (uint8_t)'-'
		&&	buffer[4] == (uint8_t)'N'
		&&	buffer[5] == (uint8_t)'e'
		&&	buffer[6] == (uint8_t)'t'
		&&	buffer[7] == 0)
	{
#if ARTNET_DEBUG_FRAME_INFO
		printf("ArtNet protocol version: %d\n", protVersion);
		printf("Frame sequence number: %u\n", seqNum);
		printf("Physical input: %u\n", physInput);
		printf("Operation Code: 0x%x\n", opCode);
		printf("Subnet: %u\n", subNet);
		printf("Universe: %u\n", universe);
		printf("Net: %u\n", net);
		printf("Length: %d\n", length);
#endif

		if (opCode == ARTNET_OPCODE_OPOUTPUT)
		{
			/* set pointer to LED data */
			*ledDataPtr = &buffer[18];
			*dataLength = length;
			retVal = ESP_OK;
		}
		else
		{
			printf("Artnet: wrong opcode\n");
		}
	}
	else
	{
		printf("Artnet: wrong ID\n");
	}

	return retVal;
}


void ArtNet__SendLedDataToUart1 (const uint8_t *data, const uint16_t length)
{
	gpio_set_level(UC_CTRL_LED_GPIO, 0);

	/* copy LED data with header */
	ledTable[0] = UART_LED_FIRST_BYTE;
	memcpy(&ledTable[1], data, length);

#if ARTNET_DEBUG_FRAME_INFO
	printf("LED data to be written: ");

	for (uint16_t dataIt = 0; dataIt < length; dataIt++)
	{
		printf(" %d", ledTable[dataIt + 1]);
	}

	printf("\n");
#endif
	/* send to UART1 */
	uart_write_bytes(UART_NUM_1, (char*)&ledTable[0], LED_TABLE_ARRAY_LENGTH);

	gpio_set_level(UC_CTRL_LED_GPIO, 1);
}


void ArtNet__Recv (void *arg, struct udp_pcb *pcb, struct pbuf *udpBuffer, const ip_addr_t *addr, uint16_t port)
{
	const uint16_t length = udpBuffer->len;
	uint8_t *ledData;
	uint16_t ledDataLength;

	if (udpBuffer != NULL)
	{
		memcpy(udpData, udpBuffer->payload, length);

#if ARTNET_DEBUG_FRAME_INFO
		printf("New UDP data - buffer length: %d\n", length);


		printf("Buffer data:");

		for (uint16_t it = 0; it < length; it++)
		{
			printf(" {%u,%u}", it, udpData[it]);
		}

		printf("\n");
#endif

		if (ArtNet__DecodeDmxFrame(udpData, &ledData, &ledDataLength) == ESP_OK)
		{
			ArtNet__SendLedDataToUart1(ledData, ledDataLength);
		}

#if ARTNET_DEBUG_FRAME_INFO
		printf("\n");
#endif
		pbuf_free(udpBuffer);
	}
}

esp_err_t ArtNet__init (void)
{
	esp_err_t retVal = ESP_FAIL;
	struct udp_pcb* pcb;

	pcb = udp_new();

	if (pcb != NULL)
	{
		if (udp_bind(pcb, &ip_addr_any, ARTNET_PORT) == ERR_OK)
		{
			udp_recv(pcb, ArtNet__Recv, NULL);
			printf("ArtNet__init OK\n");
			retVal = ESP_OK;
		}
		else
		{
			printf("ArtNet__init failed: udp_bind\n");
		}
	}
	else
	{
		printf("ArtNet__init failed: udp_new\n");
	}

	return retVal;
}
