/*
 * Mode_ColorCalibration.c
 *
 *  Created on: 24.05.2018
 *      Author: Jean-Martin George
 */


#include "Mode_ColorCalibration.h"


void ColorCalibration__init (void)
{

}


void ColorCalibration__x10 (void)
{
	LEDMatrix__setRGBColorForMatrix(LEDMatrix__getRGBColorFromComponents(255, 255, 255));
}
