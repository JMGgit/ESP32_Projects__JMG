/*
 * Main_Tasks.h
 *
 *  Created on: 08.04.2018
 *      Author: Jean-Martin George
 */

#ifndef MAIN_TASKS_H_
#define MAIN_TASKS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "Main_Config.h"
#include "ArtNet.h"
#include "LedController.h"
#include "FOTA.h"
#include "Drivers.h"
#include "Clock.h"


extern TaskHandle_t taskHandle_LedTable__mainFunction;
extern TaskHandle_t taskHandle_ArtNet__mainFunction;
extern TaskHandle_t taskHandle_ArtNet__debug;
extern TaskHandle_t taskHandle_LedController__mainFunction;
extern TaskHandle_t taskHandle_Clock__mainFunction;
extern TaskHandle_t taskHandle_MSGEQ7__mainFunction;
extern TaskHandle_t taskHandle_uC__mainFunction;
extern TaskHandle_t taskHandle_FOTA__mainFunction;


typedef enum
{
	FREERTOS_TASK_LEDTABLE,
	FREERTOS_TASK_ARTNET,
	FREERTOS_TASK_ARTNET_DEBUG,
	FREERTOS_TASK_LEDCONTROLLER,
	FREERTOS_TASK_CLOCK,
	FREERTOS_TASK_MSGEQ7,
	FREERTOS_TASK_UC,
	FREERTOS_TASK_FOTA,
	FREERTOS_TASK_NB
} freeRTOStasksIds_n;


typedef struct
{
	TaskFunction_t taskFunction;
	const char * const name;
	const uint32_t stackDepth;
	void * const parameters;
	UBaseType_t priority;
	TaskHandle_t * const taskHandle;
	const BaseType_t coreId;
} freeRtosTaskConfig_t;


freeRtosTaskConfig_t freeRTOStaskConfig[FREERTOS_TASK_NB];


void Main__deleteTask (TaskHandle_t taskHandle);
void Main__createTask (freeRTOStasksIds_n freeRTOStaskId);
void Main__createAllTasks (void);


#endif /* MAIN_INCLUDE_MAIN_TASKS_H_ */
