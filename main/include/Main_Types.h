/*
 * Main_Types.h
 *
 *  Created on: 26.01.2013
 *      Author: Jean-Martin George
 */

#ifndef MAIN_TYPES_H_
#define MAIN_TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Stubs.h"


#define FALSE	0x00
#define TRUE	0x01

#define E_OK		0
#define E_PENDING   1
#define E_NOT_OK	2

#define MIN(a, b) (((a) < (b)) ? (a) : (b) )
#define MAX(a, b) (((a) > (b)) ? (a) : (b) )

#define CONCAT(a, b)            a ## b
#define CONCAT_EXP(a, b)   CONCAT(a, b)

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} RGB_Color_t;


#endif /* MAIN_TYPES_H_ */
