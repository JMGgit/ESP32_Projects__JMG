/*
 * Mode_ColorCalibration.h
 *
 *  Created on: 24.05.2018
 *      Author: Jean-Martin George
 */

#ifndef MODE_COLORCALIBRATION_H_
#define MODE_COLORCALIBRATION_H_


#include "Main_Types.h"
#include "Main_Config.h"
#include "LEDMatrix.h"


void ColorCalibration__init (void);
void ColorCalibration__x10 (void);


#define COLOR_CALIBRATION_STEP	5

#endif /* MODE_COLORCALIBRATION_H_ */
