/*
 * STMRTOSconfig.h
 *
 */

#ifndef STMRTOSCONFIG_H_
#define STMRTOSCONFIG_H_

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"


// Pinout Definitions
#define ADC_PORT                 GPIOC
#define ADC_PIN                  GPIO_Pin_3

#define SHIFT_REG_1_PORT         GPIOE
#define SHIFT_REG_1_PIN          GPIO_Pin_2
#define SHIFT_REG_CLK_1_PIN      GPIO_Pin_1

#define SHIFT_REG_2_PORT         GPIOE
#define SHIFT_REG_2_PIN          GPIO_Pin_3
#define SHIFT_REG_CLK_2_PIN      GPIO_Pin_1

#define SHIFT_REG_3_PORT         GPIOE
#define SHIFT_REG_3_PIN          GPIO_Pin_4
#define SHIFT_REG_CLK_3_PIN      GPIO_Pin_1

#define TRAFFIC_LIGHT_PORT       GPIOD
#define TRAFFIC_LIGHT_RED_PIN    GPIO_Pin_6
#define TRAFFIC_LIGHT_YELLOW_PIN GPIO_Pin_4
#define TRAFFIC_LIGHT_GREEN_PIN  GPIO_Pin_2

// Traffic light task priorities
#define TRAFFIC_FLOW_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1 )
#define TRAFFIC_CREATE_TASK_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define TRAFFIC_LIGHT_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define TRAFFIC_DISPLAY_TASK_PRIORITY	( tskIDLE_PRIORITY  )

// Mutexes for queues
SemaphoreHandle_t	xMutexFlow;
SemaphoreHandle_t	xMutexLight;
SemaphoreHandle_t	xMutexCars;

// Queues
QueueHandle_t xQueueFlowRate;
QueueHandle_t xQueueLightColour;
QueueHandle_t xQueueCarValue;

#endif /* STMRTOSCONFIG_H_ */
