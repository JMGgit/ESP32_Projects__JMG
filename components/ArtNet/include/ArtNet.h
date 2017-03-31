/*
 * ArtNet.h
 *
 *  Created on: 02.01.2017
 *      Author: Jean-Martin George
 */

#ifndef ARTNET_H_
#define ARTNET_H_


#include <stddef.h>
#include "esp_err.h"

/* port specified by ArtNet protocol */
#define ARTNET_PORT		0x1936

/* according to protocol: 512 */
#define ARTNET_CHANNELS_PER_UNIVERSE	510

#define ARTNET_RX_UDP_FIFO_BUFFER_SIZE	200

/* ArtDmx package: max 512 channels + 18 bytes */
#define ARTNET_RX_MAX_DATA_LENGTH		(ARTNET_CHANNELS_PER_UNIVERSE + 18)

/* ArtPollReply: 178 bytes */
#define ARTNET_TX_DATA_LENGTH			239

/* Op codes supported */
#define ARTNET_OPCODE_POLL				0x2000
#define ARTNET_OPCODE_POLLREPLY			0x2100
#define ARTNET_OPCODE_DMX				0x5000

#define ARTNET_PROTOCOL_VERSION			14

#define ARTNET_NET						0
#define ARTNET_SUBNET					0
#define ARTNET_UNIVERSE_NB				((LEDS_CHANNELS / ARTNET_CHANNELS_PER_UNIVERSE) + 1)
#define ARTNET_LAST_UNIVERSE			(ARTNET_UNIVERSE_NB - 1)
#define ARTNET_FRAMECOUNTER_MAX			255

/* time after which the ArtNet component will be considered as inactive if no udp frame is received */
#define ARTNET_MAX_IDLE_TIME_MS			(2000 / portTICK_PERIOD_MS)

/********** types *********/
typedef enum
{
	ARTNET_STATE_INIT = 0,
	ARTNET_STATE_IDLE,			/* nothing to do */
	ARTNET_STATE_RECV_DECODE,	/* decoding data */
	ARTNET_STATE_POLL_REPLY
} artNetState_t;


/********** functions **********/

esp_err_t ArtNet__init (void);
void ArtNet__mainFunction (void *param);
void ArtNet__debug (void *param);

extern artNetState_t artNetState;

static inline uint8_t ArtNet__isActive (void)
{
	return ((artNetState == ARTNET_STATE_RECV_DECODE) || (artNetState == ARTNET_STATE_POLL_REPLY));
}


#define ARTNET_DEBUG_FRAME_INFO FALSE
#define ARTNET_DEBUG_ERROR_INFO TRUE


#endif /* ARTNET_H_ */
