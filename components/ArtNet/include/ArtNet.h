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
#define ARTNET_CHANNELS_PER_UNIVERSE	24 /* test value for now */

/* ArtDmx package: max 512 channels + 18 bytes */
#define ARTNET_MAX_DATA_LENGTH			(ARTNET_CHANNELS_PER_UNIVERSE + 18)

/* Op code for data output */
#define ARTNET_OPCODE_OPOUTPUT			0x5000

#define ARTNET_PROTOCOL_VERSION			14

#define ARTNET_NET						0
#define ARTNET_SUBNET					0
#define ARTNET_UNIVERSE_NB				((NUMBER_OF_LEDS_CHANNELS / ARTNET_CHANNELS_PER_UNIVERSE) + 1)
#define ARTNET_LAST_UNIVERSE			(ARTNET_UNIVERSE_NB - 1)
#define ARTNET_FRAMECOUNTER_MAX			255


/********** types *********/
typedef enum
{
	ARTNET_STATE_INIT = 0,
	ARTNET_STATE_IDLE,			/* nothing to do */
	ARTNET_STATE_RECV_BUSY,
	//ARTNET_STATE_RECV_UDP, 		/* currently UDP data being copied */
	//ARTNET_STATE_RECV_DECODE,	/* decoding data */
	ARTNET_STATE_RECV_IDLE,		/* no data processing but LED data not complete, waiting for data */
	ARTNET_STATE_SEND_UART,
} artNetState_t;


/********** functions **********/

esp_err_t ArtNet__init (void);
void ArtNet__mainFunction (void);

#define ARTNET_DEBUG_FRAME_INFO false


#endif /* ARTNET_H_ */
