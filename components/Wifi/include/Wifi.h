/*
 * Wifi.h
 *
 *  Created on: 08.02.2017
 *      Author: Jean-Martin George
 */

#ifndef WIFI_H_
#define WIFI_H_

#include "esp_event.h"

void Wifi__init (void);
uint8_t Wifi__isConnected (void);
void Wifi__systemEvent (system_event_t *event);


#endif /* WIFI_H_ */
