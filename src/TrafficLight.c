#include "TrafficLight.h"

/* Update traffic lights */
void Green_LED_Controller_Callback(xTimerHandle xTimer)
{
	GPIO_ResetBits(TRAFFIC_LIGHT_PORT, TRAFFIC_LIGHT_GREEN_PIN);
	GPIO_SetBits(TRAFFIC_LIGHT_PORT, TRAFFIC_LIGHT_YELLOW_PIN);
	uint16_t new_light = 0x00;

	// Get Light Semaphore
	if (xSemaphoreTake(xMutexLight, (TickType_t)0) == pdTRUE)
	{
		xQueueOverwrite(xQueueLightColour, &new_light);
		xSemaphoreGive(xMutexLight);
	}
	else
		printf("xMutexLight already held \n");

	// Move to yellow light
	xTimerStart(xYellowLightSoftwareTimer, 0);
}

void Yellow_LED_Controller_Callback(xTimerHandle xTimer)
{
	GPIO_ResetBits(TRAFFIC_LIGHT_PORT, TRAFFIC_LIGHT_YELLOW_PIN);
	GPIO_SetBits(TRAFFIC_LIGHT_PORT, TRAFFIC_LIGHT_RED_PIN);

	// Move to red light
	xTimerStart(xRedLightSoftwareTimer, 0);
}

void Red_LED_Controller_Callback(xTimerHandle xTimer)
{
	GPIO_ResetBits(TRAFFIC_LIGHT_PORT, TRAFFIC_LIGHT_RED_PIN);
	GPIO_SetBits(TRAFFIC_LIGHT_PORT, TRAFFIC_LIGHT_GREEN_PIN);

	// Get Light Semaphore
	if (xSemaphoreTake(xMutexLight, (TickType_t)0) == pdTRUE)
	{
		xQueueOverwrite(xQueueLightColour, (uint16_t)1);
		xSemaphoreGive(xMutexLight);
	}
	else
		printf("xMutexLight already held \n");

	// Move to green light
	xTimerStart(xGreenLightSoftwareTimer, 0);
}

/* Handle light timers using flow rate */
void TrafficLightTask(void *pvParameters)
{
	uint16_t new_speed = 7, current_speed = 0;

	while (1)
	{
		// Get Flow Semaphore
		if (xSemaphoreTake(xMutexFlow, (TickType_t)10) == pdTRUE)
		{
			xQueueReceive(xQueueFlowRate, &new_speed, 0);
			xSemaphoreGive(xMutexFlow);
		}
		else
			printf("xMutexFlow already held\n");

		if (current_speed != new_speed)
		{
			// Change light timers
			if (xTimerIsTimerActive(xRedLightSoftwareTimer))
			{
				xTimerStop(xRedLightSoftwareTimer, 0);
				xTimerChangePeriod(xGreenLightSoftwareTimer, (5000 + 3000 * (8 - new_speed)) / portTICK_PERIOD_MS, 0);
				xTimerStop(xGreenLightSoftwareTimer, 0);
				xTimerChangePeriod(xRedLightSoftwareTimer, (3000 + 1500 * (8 - new_speed)) / portTICK_PERIOD_MS, 0);
			}
			else if (xTimerIsTimerActive(xYellowLightSoftwareTimer))
			{
				xTimerChangePeriod(xGreenLightSoftwareTimer, (5000 + 3000 * (8 - new_speed)) / portTICK_PERIOD_MS, 0);
				xTimerStop(xGreenLightSoftwareTimer, 0);
				xTimerChangePeriod(xRedLightSoftwareTimer, (3000 + 1500 * (8 - new_speed)) / portTICK_PERIOD_MS, 0);
				xTimerStop(xRedLightSoftwareTimer, 0);
			}
			else if (xTimerIsTimerActive(xGreenLightSoftwareTimer))
			{
				xTimerStop(xGreenLightSoftwareTimer, 0);
				xTimerChangePeriod(xGreenLightSoftwareTimer, (5000 + 3000 * (8 - new_speed)) / portTICK_PERIOD_MS, 0);
				xTimerChangePeriod(xRedLightSoftwareTimer, (3000 + 1500 * (8 - new_speed)) / portTICK_PERIOD_MS, 0);
				xTimerStop(xRedLightSoftwareTimer, 0);
			}
		}
		current_speed = new_speed;

		vTaskDelay(800);
	}
}
