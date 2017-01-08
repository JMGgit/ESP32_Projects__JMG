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

/* ArtDmx package: max 512 channels + 18 bytes */
#define ARTNET_MAX_DATA_LENGTH	(512+18)

#define ARTNET_OPCODE_OPOUTPUT	0x5000

esp_err_t ArtNet__init (void);

#define ARTNET_DEBUG_FRAME_INFO false


#endif /* ARTNET_H_ */
