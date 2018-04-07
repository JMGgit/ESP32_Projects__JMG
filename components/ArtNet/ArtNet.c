/*
 * ArtNet.c
 *
 *  Created on: 02.01.2017
 *      Author: Jean-Martin George
 */

#include "Main_Config.h"
#include "ArtNet.h"
#include "lwip/udp.h"
#include "lwip/debug.h"
#include "Wifi.h"
#include "LedController.h"
#include "esp_task_wdt.h"
#include <string.h>

#include "uC.h"

/******* ArtNet definitions *********
Node: DMX512 - ArtNet converter
Universe: 512 channels
Sub-Net: 16 consecutive universes
Net: 16 consecutive sub-nets
*************************************/

struct udp_pcb* pcb;

uint8_t *udpDataRx;
uint8_t udpDataRxFifo[ARTNET_RX_UDP_FIFO_BUFFER_SIZE][ARTNET_RX_MAX_DATA_LENGTH];
uint8_t udpDataReady[ARTNET_RX_UDP_FIFO_BUFFER_SIZE];
uint8_t udpDataRxFifo1[ARTNET_RX_MAX_DATA_LENGTH];
uint8_t udpFrameCounter = 0;
uint8_t udpDataTx[ARTNET_TX_DATA_LENGTH];

/* define 2 led tables for receive and send */
uint8_t artNetLedTable[LEDS_CHANNELS];

/* flag to indicate that data for each universe has been received for the current frame*/
uint16_t univDataRecv[ARTNET_FRAMECOUNTER_MAX + 1];

uint8_t ledData[LEDS_CHANNELS];

artNetState_t artNetState = ARTNET_STATE_INIT;
uint8_t newUdpDataRecv;

uint8_t *currentDmxData;
uint8_t currentUniverse;
uint16_t currentDmxDataLength;
uint8_t currentFrame = 0;

uint8_t udpFrameIterator;
uint32_t frameCounterRecv;
uint32_t missedFrameCounterRecv;
uint32_t oldFrameCounterMain;
uint32_t oldFrameCounterRecv = 0;
float errorRateRecv;

static uint8_t lastFrameDecoded = 0;
static uint8_t frameDelay = 0;
static uint8_t maxFrameDelay = 0;
static uint32_t missedFrameCounterRecv_prev = 0;
static uint32_t missedFrameCounterMain = 0;
static float errorRateMain = 0;
static uint32_t frameCounterMain = 0;


esp_err_t ArtNet__decodeDmxFrame (uint8_t *buffer, uint8_t *frameNb, uint8_t **ledDataPtr, uint16_t *ledDataLength, uint8_t *ledStart)
{
	esp_err_t retVal = ESP_FAIL;
	const uint8_t seqNum = buffer[12];
	const uint8_t physInput = buffer[13];
	const uint8_t subnet = buffer[14] / 16;
	const uint8_t universe = buffer[14] % 16;
	const uint8_t net = buffer[15];
	const uint16_t length = (buffer[16] << 8) + buffer[17];
	uint8_t error = FALSE;

	if (net == ARTNET_NET)
	{
		if (subnet == ARTNET_SUBNET)
		{
			if (universe < ARTNET_UNIVERSE_NB)
			{
				if (		((universe == ARTNET_LAST_UNIVERSE) && (length <= (LEDS_CHANNELS % ARTNET_CFG_CHANNELS_PER_UNIVERSE)))
						||	(((ARTNET_UNIVERSE_NB > 1) && (universe < ARTNET_LAST_UNIVERSE)) && (length == ARTNET_CFG_CHANNELS_PER_UNIVERSE))
				)
				{
					*frameNb = seqNum;
					*ledStart = universe;
					*ledDataLength = length;
					*ledDataPtr = &buffer[18];
				}
				else
				{
					printf("Artnet: wrong data length: max value is %d or %d for last universe\n", ARTNET_CFG_CHANNELS_PER_UNIVERSE, LEDS_CHANNELS % ARTNET_CFG_CHANNELS_PER_UNIVERSE);
					error = TRUE;
					artNetState = ARTNET_STATE_IDLE;
				}

				retVal = ESP_OK;
			}
			else
			{
				printf("Artnet: wrong universe: should be less than %d\n", ARTNET_UNIVERSE_NB);
				error = TRUE;
				artNetState = ARTNET_STATE_IDLE;
			}
		}
		else
		{
			printf("Artnet: wrong subnet: should be 0\n");
			error = TRUE;
			artNetState = ARTNET_STATE_IDLE;
		}
	}
	else
	{
		printf("Artnet: wrong net: should be 0\n");
		error = TRUE;
		artNetState = ARTNET_STATE_IDLE;
	}

	if (error || ARTNET_DEBUG_FRAME_INFO)
	{
		printf("Frame sequence number: %u\n", seqNum);
		printf("Physical input: %u\n", physInput);
		printf("Subnet: %u\n", subnet);
		printf("Universe: %u\n", universe);
		printf("Net: %u\n", net);
		printf("Length: %d\n", length);
	}

	return retVal;
}


esp_err_t ArtNet__decodePollFrame (uint8_t *buffer)
{
	/* no check necessary for now */
	return ESP_OK;
}


void ArtNet__fillArtPollReplyBuffer (uint8_t portOffset, uint8_t *buffer, uint8_t *dataLength)
{
	uint8_t idx = 0;

	/* 1: ID [8] */
	memcpy(&buffer[idx], "Art-Net", 7);
	idx = idx + 7;
	buffer[idx++] = 0;

	/* 2: OpCode [2] */
	buffer[idx++] = (uint8_t)ARTNET_OPCODE_POLLREPLY;
	buffer[idx++] = ARTNET_OPCODE_POLLREPLY >> 8;

	/* 3: IP adress [4] */
	buffer[idx++] = 192;
	buffer[idx++] = 168;
	buffer[idx++] = 2;
	buffer[idx++] = 109;

	/* 4: Port [2] */
	buffer[idx++] = (uint8_t)ARTNET_PORT;
	buffer[idx++] = ARTNET_PORT >> 8;

	/* 5: VersInfoH [1] */
	buffer[idx++] = 1;

	/* 6: VersInfoL [1] */
	buffer[idx++] = 0;

	/* 7: NetSwitch [1] */
	buffer[idx++] = 0; /* TEST VALUE */

	/* 8: SubSwitch [1] */
	buffer[idx++] = 0; /* TEST VALUE */

	/* 9: OemHi [1] */
	buffer[idx++] = 0;

	/* 10: Oem [1] */
	buffer[idx++] = 0;

	/* 11: Ubea Version [1] */
	buffer[idx++] = 0;

	/* 12: Status1 [1] */
	buffer[idx++] = 0b11010000; /* Indicators in Normal Mode. All Port Address set by front panel controls. */

	/* 13: EstaManLo */
	buffer[idx++] = 0;

	/* 14: EstManHi */
	buffer[idx++] = 0;

	/* 15: ShortName [18] */
	memcpy(&buffer[idx], "ESP32 - 01-01     ", 17);
	idx = idx + 17;
	buffer[idx++] = 0;

	/* 16: LongName [64] */
	memcpy(&buffer[idx], "ESP32 ArtNet node - 01-01                                      ", 63);
	idx = idx + 63;
	buffer[idx++] = 0;

	/* 17: NodeReport [64] */
	memset(&buffer[idx], 0, 64);
	idx = idx + 64;

	if ((ARTNET_UNIVERSE_NB - portOffset) >= 4)
	{
		/* 18: NumPortsHi */
		buffer[idx++] = 4 >> 8;

		/* 19: NumPortsLo */
		buffer[idx++] = 4;

		/* 20: PortTypes [4] */
		buffer[idx++] = 0x80; /* artnet input - dmx output */
		buffer[idx++] = 0x80; /* artnet input - dmx output */
		buffer[idx++] = 0x80; /* artnet input - dmx output */
		buffer[idx++] = 0x80; /* artnet input - dmx output */

		/* 21: GoodInput [4] */
		buffer[idx++] = 0x08; /* Port disabled */
		buffer[idx++] = 0x08; /* Port disabled */
		buffer[idx++] = 0x08; /* Port disabled */
		buffer[idx++] = 0x08; /* Port disabled */

		/* 22: GoodOutput [4] */
		buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
		buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
		buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
		buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */

		/* 23: SwIn [4] */
		buffer[idx++] = 0;
		buffer[idx++] = 0;
		buffer[idx++] = 0;
		buffer[idx++] = 0;

		/* 24: SwOut [4] */
		buffer[idx++] = portOffset;
		buffer[idx++] = portOffset + 1;
		buffer[idx++] = portOffset + 2;
		buffer[idx++] = portOffset + 3;
	}
	else
	{
		/* 18: NumPortsHi */
		buffer[idx++] = (ARTNET_UNIVERSE_NB - portOffset) >> 8;

		/* 19: NumPortsLo */
		buffer[idx++] = ARTNET_UNIVERSE_NB - portOffset;

		if ((ARTNET_UNIVERSE_NB - portOffset) == 3)
		{
			/* 20: PortTypes [4] */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */

			/* 21: GoodInput [4] */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */

			/* 22: GoodOutput [4] */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */

			/* 23: SwIn [4] */
			buffer[idx++] = 0;
			buffer[idx++] = 0;
			buffer[idx++] = 0;
			buffer[idx++] = 0;

			/* 24: SwOut [4] */
			buffer[idx++] = portOffset;
			buffer[idx++] = portOffset + 1;
			buffer[idx++] = portOffset + 2;
			buffer[idx++] = 0;
		}
		else if ((ARTNET_UNIVERSE_NB - portOffset) == 2)
		{
			/* 20: PortTypes [4] */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */

			/* 22: GoodOutput [4] */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */

			/* 21: GoodInput [4] */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */

			/* 23: SwIn [4] */
			buffer[idx++] = 0;
			buffer[idx++] = 0;
			buffer[idx++] = 0;
			buffer[idx++] = 0;

			/* 24: SwOut [4] */
			buffer[idx++] = portOffset;
			buffer[idx++] = portOffset + 1;
			buffer[idx++] = 0;
			buffer[idx++] = 0;
		}
		else if ((ARTNET_UNIVERSE_NB - portOffset) == 1)
		{
			/* 20: PortTypes [4] */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */
			buffer[idx++] = 0x80; /* artnet input - dmx output */

			/* 21: GoodInput [4] */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */
			buffer[idx++] = 0x08; /* Port disabled */

			/* 22: GoodOutput [4] */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */
			buffer[idx++] = (1 << 7); /* transmitting artnet - dmx data */

			/* 23: SwIn [4] */
			buffer[idx++] = 0;
			buffer[idx++] = 0;
			buffer[idx++] = 0;
			buffer[idx++] = 0;

			/* 24: SwOut [4] */
			buffer[idx++] = portOffset;
			buffer[idx++] = 0;
			buffer[idx++] = 0;
			buffer[idx++] = 0;
		}
		else
		{
			/* will not happen */
		}
	}

	/* 25 SwVideo [1] */
	buffer[idx++] = 0;

	/* 26 SwMacro [1] */
	buffer[idx++] = 0;

	/* 27 SwRemote [1] */
	buffer[idx++] = 0;

	/* 28 Spare [1] */
	buffer[idx++] = 0;

	/* 29 Spare [1] */
	buffer[idx++] = 0;

	/* 30 Spare [1] */
	buffer[idx++] = 0;

	/* 31 Style [1] */
	buffer[idx++] = 0;

	/* 32 MAC Hi [1] */
	buffer[idx++] = 0x24;

	/* 33 MAC [1] */
	buffer[idx++] = 0x0a;

	/* 34 MAC [1] */
	buffer[idx++] = 0xc4;

	/* 35 MAC [1] */
	buffer[idx++] = 0x00;

	/* 36 MAC [1] */
	buffer[idx++] = 0x9f;

	/* 37 MAC Lo [1] */
	buffer[idx++] = 0x90;

	/* 38 BinIp [4] */
	buffer[idx++] = 192;
	buffer[idx++] = 168;
	buffer[idx++] = 2;
	buffer[idx++] = 109;

	/* 39 BindIndex [1] */
	buffer[idx++] = 1;

	/* 40 Status2 [1] */
	buffer[idx++] = 0b00000110; /* DHCP capable - addresses 15-bit */

	/* 41 Filler [26] */
	memset(&buffer[idx], 0, 26);
	idx = idx + 26;

	*dataLength = idx;
}


esp_err_t ArtNet__sendPollReply (struct udp_pcb *pcb, uint8_t *data, uint8_t dataLength)
{
	esp_err_t retVal = ESP_FAIL;
	struct pbuf *udpBuffer;

	udpBuffer = pbuf_alloc(PBUF_TRANSPORT, dataLength, PBUF_RAM);
	memcpy (udpBuffer->payload, data, dataLength);
	udp_sendto(pcb, udpBuffer, IP_ADDR_BROADCAST, ARTNET_PORT);
	pbuf_free(udpBuffer);

	/* debug infos */
	printf("ArtNet poll reply: %d bytes sent\n", dataLength);

	for (int i = 0; i < dataLength; i++)
	{
		printf("%d ", data[i]);
	}

	printf("\n");

	retVal  = ESP_OK;

	return retVal;
}


void ArtNet__setDataAvailable (const uint8_t frameNb, uint8_t universe)
{
	univDataRecv[frameNb] |= (1 << universe);
}


uint8_t ArtNet__allDataAvailable (uint8_t frameNb)
{
	return (univDataRecv[frameNb] == ((1 << ARTNET_UNIVERSE_NB) - 1));
}


void ArtNet__recvUdpFrame (void *arg, struct udp_pcb *pcb, struct pbuf *udpBuffer, const ip_addr_t *addr, uint16_t port)
{
	static uint8_t frameNb;
	static uint8_t sameFrameCounter = 0;
	const uint16_t length = udpBuffer->len;

	gpio_set_level(TEST_LED_ARTNET_GPIO, 1);

	if (udpBuffer != NULL)
	{
		if (length <= ARTNET_RX_MAX_DATA_LENGTH)
		{
			if (udpFrameCounter < (ARTNET_RX_UDP_FIFO_BUFFER_SIZE - 1))
			{
				udpFrameCounter++;
			}
			else
			{
				udpFrameCounter = 0;
			}

			udpDataReady[udpFrameCounter] = 0;
			memcpy(&udpDataRxFifo[udpFrameCounter][0], udpBuffer->payload, length);

			if (artNetState != ARTNET_STATE_RECV_DECODE)
			{
				newUdpDataRecv = TRUE;
			}

			if (udpDataRxFifo[udpFrameCounter][12] == frameNb)
			{
				if (sameFrameCounter < (ARTNET_UNIVERSE_NB - 1))
				{
					sameFrameCounter++;
				}
				else
				{
					sameFrameCounter = 0;
				}
			}
			else
			{
				if ((udpDataRxFifo[udpFrameCounter][12] != 1) && (udpDataRxFifo[udpFrameCounter][12] < frameNb))
				{
					oldFrameCounterRecv++;
				}

				if (sameFrameCounter != 0)
				{
					missedFrameCounterRecv++;
				}

				frameNb = udpDataRxFifo[udpFrameCounter][12];
				frameCounterRecv++;
				sameFrameCounter = 1;
			}

			udpDataReady[udpFrameCounter] = 1;


#if ARTNET_DEBUG_FRAME_INFO
			printf("New UDP data - buffer length: %d\n", length);
			printf("Buffer data:");

			for (uint16_t it = 0; it < 20; it++)
			{
				printf(" {%u,%u}", it, udpDataRxFifo1[it]);
			}

			printf("\n");
#endif

		}
		else
		{
			printf("UDP frame size exceed ArtNet buffer length\n");
		}

		pbuf_free(udpBuffer);
	}

	gpio_set_level(TEST_LED_ARTNET_GPIO, 0);
}


esp_err_t ArtNet__init (void)
{
    uint8_t univIt;

    esp_err_t retVal = ESP_FAIL;

    if (Wifi__isConnected())
    {
        /* re init universe table */
        for (univIt = 0; univIt < ARTNET_UNIVERSE_NB; univIt++)
        {
            univDataRecv[univIt] = 0;
        }

        pcb = udp_new();

        if (pcb != NULL)
        {
            if (udp_bind(pcb, &ip_addr_any, ARTNET_PORT) == ERR_OK)
            {
                udp_recv(pcb, ArtNet__recvUdpFrame, NULL);
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

        gpio_set_direction(TEST_LED_ARTNET_GPIO, GPIO_MODE_OUTPUT);
    }

    return retVal;
}


void ArtNet__mainFunction (void *param)
{
	uint16_t opCode = 0;
	uint16_t protVersion = 0;
	uint8_t newFrame = 0;
	uint8_t TxDataLength = 0;
	uint8_t artPollReply_portOffset = 0;
	uint8_t stateTransition = TRUE;
	static uint8_t noUdpFrameTransition = FALSE;
	static TickType_t tickNoUdpFrameCurrent;
	static TickType_t tickNoUdpFrameTransition;

	while (1)
	{
        if (!Wifi__isConnected())
        {
            artNetState = ARTNET_STATE_INIT;
        }
        else if (newUdpDataRecv)
		{
			artNetState = ARTNET_STATE_RECV_DECODE;
			stateTransition = TRUE;
			newUdpDataRecv = FALSE;
		}
		else
		{
			stateTransition = FALSE;
		}

        switch (artNetState)
		{
			case ARTNET_STATE_INIT:
			{
				if (ArtNet__init() == ESP_OK)
				{
					artNetState = ARTNET_STATE_IDLE;
				}

				break;
			}

			case ARTNET_STATE_RECV_DECODE:
			{
				if (stateTransition)
				{
					udpFrameIterator = udpFrameCounter;
					printf("ArtNet DMX mode\n\n");
				}
				else if (udpFrameIterator == udpFrameCounter)
				{
					/* wait */
				}
				else if (udpFrameIterator < (ARTNET_RX_UDP_FIFO_BUFFER_SIZE - 1))
				{
					udpFrameIterator++;
				}
				else
				{
					udpFrameIterator = 0;
				}

				if ((udpDataReady[udpFrameIterator]) && (lastFrameDecoded != udpFrameIterator))
				{
					udpDataRx = &udpDataRxFifo[udpFrameIterator][0];

					opCode = (udpDataRx[9] << 8) + udpDataRx[8];
					protVersion = (udpDataRx[10] << 8) + udpDataRx[11];

					if (	udpDataRx[0] == (uint8_t)'A'
						&&	udpDataRx[1] == (uint8_t)'r'
						&&	udpDataRx[2] == (uint8_t)'t'
						&&	udpDataRx[3] == (uint8_t)'-'
						&&	udpDataRx[4] == (uint8_t)'N'
						&&	udpDataRx[5] == (uint8_t)'e'
						&&	udpDataRx[6] == (uint8_t)'t'
						&&	udpDataRx[7] == 0)
					{
#if ARTNET_DEBUG_FRAME_INFO
						printf("ArtNet protocol version: %d\n", protVersion);
						printf("Operation Code: 0x%x\n", opCode);
#endif

						if (protVersion == ARTNET_PROTOCOL_VERSION)
						{
							if (opCode == ARTNET_OPCODE_DMX)
							{
								if (ArtNet__decodeDmxFrame(udpDataRx, &newFrame, &currentDmxData, &currentDmxDataLength, &currentUniverse) == ESP_OK)
								{
									if ((newFrame >= currentFrame) || (newFrame == 1))
									{
										if (newFrame != currentFrame)
										{
											if (ArtNet__allDataAvailable(currentFrame))
											{
												while (ESP_OK != LedController__outputLedData())
												{
													/* wait until data are sent */
												}
											}
											else
											{
												missedFrameCounterMain++;
											}

											univDataRecv[currentFrame] = 0;
											frameCounterMain++;
										}

										currentFrame = newFrame;
										ArtNet__setDataAvailable(currentFrame, currentUniverse);

										while (ESP_OK != LedController__storeLedData(currentDmxData,  currentUniverse * ARTNET_CFG_CHANNELS_PER_UNIVERSE, currentDmxDataLength))
										{
											/* wait until data are sent */
										}
									}
									else
									{
										/* wrong frame number (old frame) -> re init universe table and reset state */
										univDataRecv[currentFrame] = 0;
										missedFrameCounterMain++;
										oldFrameCounterMain++;
									}
								}
							}
							else if (opCode == ARTNET_OPCODE_POLL)
							{
								if (ArtNet__decodePollFrame(udpDataRx) == ESP_OK)
								{
									printf("Artnet: decode poll ok\n");
									artNetState = ARTNET_STATE_POLL_REPLY;
								}
								else
								{
									printf("Artnet: decode poll failed\n");
									artNetState = ARTNET_STATE_IDLE;
								}
							}
							else
							{
								printf("Artnet: wrong opcode\n");
								artNetState = ARTNET_STATE_IDLE;
							}
						}
						else
						{
							printf("Artnet: wrong protocol version: should be %d. Detected: %d\n", ARTNET_PROTOCOL_VERSION, protVersion);
							artNetState = ARTNET_STATE_IDLE;
						}
					}
					else
					{
						printf("Artnet: wrong ID\n");
						artNetState = ARTNET_STATE_IDLE;
					}

					lastFrameDecoded = udpFrameIterator;
					noUdpFrameTransition = FALSE;
				}
				else
				{
					if (noUdpFrameTransition == FALSE)
					{
						tickNoUdpFrameTransition = xTaskGetTickCount();
						noUdpFrameTransition = TRUE;
					}
					else
					{
						tickNoUdpFrameCurrent  = xTaskGetTickCount();

						if ((tickNoUdpFrameCurrent - tickNoUdpFrameTransition) > ARTNET_MAX_IDLE_TIME_MS)
						{
							printf("ArtNet set to inactive after ARTNET_MAX_IDLE_TIME_MS\n\n");
							artNetState = ARTNET_STATE_IDLE;
						}
					}
				}

				if ((udpFrameCounter - udpFrameIterator) >= 0)
				{
					frameDelay = udpFrameCounter - udpFrameIterator;
				}
				else
				{
					frameDelay = udpFrameCounter + 255 - udpFrameIterator;
				}

				if (frameDelay > maxFrameDelay)
				{
					maxFrameDelay = frameDelay;
				}

				break;
			}

			case ARTNET_STATE_POLL_REPLY:
			{
				while (artPollReply_portOffset < ARTNET_UNIVERSE_NB)
				{
					ArtNet__fillArtPollReplyBuffer(artPollReply_portOffset, udpDataTx, &TxDataLength);
					ArtNet__sendPollReply(pcb, udpDataTx, TxDataLength);
					artPollReply_portOffset = artPollReply_portOffset + 4;
				}

				printf("ArtPollReply: %d message(s) sent\n\n", artPollReply_portOffset / 4);

				artPollReply_portOffset = 0;
				artNetState = ARTNET_STATE_IDLE;

				break;
			}

			default:
			{
				break;
			}
		}

        /* statistics for Artnet__debug */
		errorRateRecv = (float)(missedFrameCounterRecv * 100) / (float)frameCounterRecv;
		errorRateMain = (float)(missedFrameCounterMain * 100) / (float)frameCounterMain;
		missedFrameCounterRecv_prev = missedFrameCounterRecv;

		/* reset watchdog */
		esp_task_wdt_reset();
	}
}


void ArtNet__debug (void *param)
{
	while (1)
	{
#if ARTNET_DEBUG_ERROR_INFO
		if (ArtNet__isActive())
		{
			printf("\n");
			printf("Missed frames counter (Interrupt): %d - total frames: %d - Error rate: %f %% - old frames: %d\n", missedFrameCounterRecv, frameCounterRecv, errorRateRecv, oldFrameCounterRecv);
			printf("Missed frames counter (MainFunction): %d - total frames: %d - Error rate: %f %% - old frames: %d\n", missedFrameCounterMain, frameCounterMain, errorRateMain, oldFrameCounterMain);
			printf("Frame delay: %d (max: %d)\n", frameDelay, maxFrameDelay);
		}
#endif
		vTaskDelay(10000 / portTICK_PERIOD_MS);

		/* reset watchdog */
		esp_task_wdt_reset();
	}
}


void ArtNet__shutdown (void)
{
	udp_remove(pcb);
	printf("ArtNet__shutdown done\n");
}
