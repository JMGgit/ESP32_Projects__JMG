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

/******* ArtNet definitions *********
Node: DMX512 - ArtNet converter
Universe: 512 channels
Sub-Net: 16 consecutive universes
Net: 16 consecutive sub-nets
*************************************/


uint8_t udpData[ARTNET_MAX_DATA_LENGTH];
uint8_t artNetDataReady;

/* define 2 led tables for receive and send */
uint8_t ledTable_1[LED_TABLE_ARRAY_LENGTH];
uint8_t ledTable_2[LED_TABLE_ARRAY_LENGTH];

uint8_t *ledTablePtr_Recv;
uint8_t *ledTablePtr_Send;

/* flag to indicate that data for each universe has been received */
uint8_t univDataRecv[ARTNET_UNIVERSE_NB];


void ArtNet__swapRcvSendTables (void)
{
	uint8_t univIt;
	uint8_t *tempPtr = &ledTablePtr_Send[0];

	ledTablePtr_Send = &ledTablePtr_Recv[0];
	ledTablePtr_Recv = tempPtr;

	/* re init universe table */
	for (univIt = 0; univIt < ARTNET_UNIVERSE_NB; univIt++)
	{
		univDataRecv[univIt] = false;
	}
}


esp_err_t ArtNet__decodeDmxFrame (uint8_t *buffer, uint8_t **ledDataPtr, uint16_t *ledDataLength, uint8_t *ledStart)
{
	esp_err_t retVal = ESP_FAIL;

	const uint16_t opCode = (buffer[9] << 8) + buffer[8];
	const uint16_t protVersion = (buffer[10] << 8) + buffer[11];
#if ARTNET_DEBUG_FRAME_INFO
	const uint8_t seqNum = buffer[12];
	const uint8_t physInput = buffer[13];
#endif
	const uint8_t subnet = buffer[14] / 16;
	const uint8_t universe = buffer[14] % 16;
	const uint8_t net = buffer[15];
	const uint16_t length = (buffer[16] << 8) + buffer[17];


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
		printf("Subnet: %u\n", subnet);
		printf("Universe: %u\n", universe);
		printf("Net: %u\n", net);
		printf("Length: %d\n", length);
#endif

		if (protVersion == ARTNET_PROTOCOL_VERSION)
		{
			if (opCode == ARTNET_OPCODE_OPOUTPUT)
			{
				if (net == ARTNET_NET)
				{
					if (subnet == ARTNET_SUBNET)
					{
						if (universe < ARTNET_UNIVERSE_NB)
						{
							if (		((universe == ARTNET_LAST_UNIVERSE) && (length == (NUMBER_OF_LEDS_CHANNELS % ARTNET_CHANNELS_PER_UNIVERSE)))
									||	((universe < ARTNET_LAST_UNIVERSE) && (length <= ARTNET_CHANNELS_PER_UNIVERSE))
							)
							{
								*ledStart = universe;
								*ledDataLength = (buffer[16] << 8) + buffer[17];
								*ledDataPtr = &buffer[18];
							}
							else
							{
								printf("Artnet: wrong data length: should be less than %d or %d for last universe\n", ARTNET_CHANNELS_PER_UNIVERSE, NUMBER_OF_LEDS_CHANNELS % ARTNET_CHANNELS_PER_UNIVERSE);
							}

							retVal = ESP_OK;
						}
						else
						{
							printf("Artnet: wrong universe: should be less than %d\n", ARTNET_UNIVERSE_NB);
						}
					}
					else
					{
						printf("Artnet: wrong subnet: should be 0\n");
					}
				}
				else
				{
					printf("Artnet: wrong net: should be 0\n");
				}
			}
			else
			{
				printf("Artnet: wrong opcode\n");
			}
		}
		else
		{
			printf("Artnet: wrong protocol version: should be %d\n", ARTNET_PROTOCOL_VERSION);
		}
	}
	else
	{
		printf("Artnet: wrong ID\n");
	}

	return retVal;
}


void ArtNet__storeLedData (const uint8_t *data, const uint16_t length, uint8_t universe)
{
	uint8_t univIt;
	uint8_t allUnivDataRcv = true;

	memcpy(&ledTablePtr_Recv[(universe * ARTNET_CHANNELS_PER_UNIVERSE)], data, length);
	univDataRecv[universe] = true;

#if ARTNET_DEBUG_FRAME_INFO
	printf("Data for universes available:");
#endif

	for (univIt = 0; univIt < ARTNET_UNIVERSE_NB; univIt++)
	{
		allUnivDataRcv &= univDataRecv[univIt];
#if ARTNET_DEBUG_FRAME_INFO
		printf(" %d", univDataRecv[univIt]);
#endif
	}

#if ARTNET_DEBUG_FRAME_INFO
	printf("\n");
#endif

	if (allUnivDataRcv)
	{
		artNetDataReady = true;
	}
}


void ArtNet__sendLedDataToUart1 (void)
{
	uint8_t uartFirstByte = 1;

	gpio_set_level(UC_CTRL_LED_GPIO, 1);

	uart_write_bytes(UART_NUM_1, (char*)&uartFirstByte, 1);
	uart_write_bytes(UART_NUM_1, (char*)&ledTablePtr_Send[0], LED_TABLE_ARRAY_LENGTH);

	gpio_set_level(UC_CTRL_LED_GPIO, 0);

#if ARTNET_DEBUG_FRAME_INFO
	printf("LED data sent to UART1: %d ", uartFirstByte);

	for (uint16_t dataIt = 0; dataIt < LED_TABLE_ARRAY_LENGTH; dataIt++)
	{
		printf(" %d", ledTablePtr_Send[dataIt]);
	}

	printf("\n");
#endif
}


void ArtNet__recv (void *arg, struct udp_pcb *pcb, struct pbuf *udpBuffer, const ip_addr_t *addr, uint16_t port)
{
	const uint16_t length = udpBuffer->len;
	uint8_t *ledData;
	uint8_t ledDataStart;
	uint16_t ledDataLength;

	gpio_set_level(UC_TEST1_GPIO, 1);

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

		if (ArtNet__decodeDmxFrame(udpData, &ledData, &ledDataLength, &ledDataStart) == ESP_OK)
		{
			ArtNet__storeLedData(ledData, ledDataLength, ledDataStart);
		}

#if ARTNET_DEBUG_FRAME_INFO
		printf("\n");
#endif
		pbuf_free(udpBuffer);
	}

	gpio_set_level(UC_TEST1_GPIO, 0);
}


esp_err_t ArtNet__init (void)
{
	uint8_t univIt;

	esp_err_t retVal = ESP_FAIL;
	struct udp_pcb* pcb;

	artNetDataReady = false;

	/* init pointer to table */
	ledTablePtr_Recv = &ledTable_1[0];
	ledTablePtr_Send = &ledTable_2[0];

	/* re init universe table */
	for (univIt = 0; univIt < ARTNET_UNIVERSE_NB; univIt++)
	{
		univDataRecv[univIt] = false;
	}

	pcb = udp_new();

	if (pcb != NULL)
	{
		if (udp_bind(pcb, &ip_addr_any, ARTNET_PORT) == ERR_OK)
		{
			udp_recv(pcb, ArtNet__recv, NULL);
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


void ArtNet__mainFunction (void)
{
	if (artNetDataReady)
	{
#if ARTNET_DEBUG_FRAME_INFO
		printf("ArtNet data ready!!\n");
#endif
		artNetDataReady = false;
		ArtNet__swapRcvSendTables();
		ArtNet__sendLedDataToUart1();
	}
}

