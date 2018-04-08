/*
 * Main_Tasks.c
 *
 *  Created on: 08.04.2018
 *      Author: JMG
 */


#include "Main_Tasks.h"


TaskHandle_t taskHandle_LedTable__mainFunction;
TaskHandle_t taskHandle_ArtNet__mainFunction;
TaskHandle_t taskHandle_ArtNet__debug;
TaskHandle_t taskHandle_LedController__mainFunction;
TaskHandle_t taskHandle_Clock__mainFunction;
TaskHandle_t taskHandle_MSGEQ7__mainFunction;
TaskHandle_t taskHandle_uC__mainFunction;
TaskHandle_t taskHandle_FOTA__mainFunction;

extern void LedTable__mainFunction (void *param);


freeRtosTaskConfig_t freeRTOStaskConfig[] =
{
		/* taskFunction					name							stackDepth	parameters 	priority	taskHandle									coreId*/
		{LedTable__mainFunction,		"LedTable__mainFunction",		4096,		NULL,			1,		&taskHandle_LedTable__mainFunction,			tskNO_AFFINITY},
		{ArtNet__mainFunction,			"ArtNet__mainFunction",			4096,		NULL,			1,		&taskHandle_ArtNet__mainFunction,			tskNO_AFFINITY},
		{ArtNet__debug,					"ArtNet__debug",				4096,		NULL,			10,		&taskHandle_ArtNet__debug,					tskNO_AFFINITY},
		{LedController__mainFunction,	"LedController__mainFunction",	4096,		NULL,			1,		&taskHandle_LedController__mainFunction,	tskNO_AFFINITY},
		{Clock__mainFunction,			"Clock__mainFunction",			4096,		NULL,			2,		&taskHandle_Clock__mainFunction,			tskNO_AFFINITY},
		{MSGEQ7__mainFunction,			"MSGEQ7__mainFunction",			4096,		NULL,			3,		&taskHandle_MSGEQ7__mainFunction,			tskNO_AFFINITY},
		{uC__mainFunction,				"uC__mainFunction",				4096,		NULL,			1,		&taskHandle_uC__mainFunction,				tskNO_AFFINITY},
		{FOTA__mainFunction,			"FOTA__mainFunction",			8192,		NULL,			1,		&taskHandle_FOTA__mainFunction,				tskNO_AFFINITY},
};


void Main__createTask (freeRTOStasksIds_n taskId)
{
	if (pdPASS == xTaskCreatePinnedToCore(
			freeRTOStaskConfig[taskId].taskFunction,
			freeRTOStaskConfig[taskId].name,
			freeRTOStaskConfig[taskId].stackDepth,
			freeRTOStaskConfig[taskId].parameters,
			freeRTOStaskConfig[taskId].priority,
			freeRTOStaskConfig[taskId].taskHandle,
			freeRTOStaskConfig[taskId].coreId)
	)
	{
		esp_task_wdt_add(taskHandle_LedTable__mainFunction);
		printf("Task LedTable__mainFunction created\n");
	}
}


void Main__createAllTasks (void)
{
	freeRTOStasksIds_n taskIt;

	for (taskIt = 0; taskIt < FREERTOS_TASK_NB; taskIt++)
	{
		Main__createTask(taskIt);
	}
}


void Main__deleteTask (TaskHandle_t taskHandle)
{
	esp_task_wdt_delete(taskHandle);
	vTaskDelete(taskHandle);
}
