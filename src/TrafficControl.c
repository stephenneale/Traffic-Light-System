/*
 * Traffic.c
 *
 */

#include <TrafficControl.h>

/* Reads in ADC value, converts to discrete 0-7 integer, adjusts traffic flow accordingly */
void TrafficFlowAdjustmentTask(void *pvParameters)
{
	uint16_t adc_value = 0, speed_adc_value = 0, current_speed_value = 0;
	uint16_t speed_change;

	while (1)
	{
		printf("Start Traffic Flow Adjustment Task\n");

		// Begin ADC
		ADC_SoftwareStartConv(ADC1);

		// Wait for conversion
		while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
			;
		adc_value = ADC_GetConversionValue(ADC1);

		// Convert ADC to 0-7
		speed_adc_value = adc_value / 512;
		if (speed_adc_value == 8)
			speed_adc_value = 7;

		// Calculate speed difference
		speed_change = abs(speed_adc_value - current_speed_value);

		// Update values if ADC value changes
		if (speed_change)
		{
			current_speed_value = speed_adc_value;

			// Grab Flow Semaphore
			if (xSemaphoreTake(xMutexFlow, (TickType_t)10) == pdTRUE)
			{
				xQueueOverwrite(xQueueFlowRate, &speed_adc_value);
				xSemaphoreGive(xMutexFlow);
			}
			else
				printf("xMutexFlow already held\n");
		}

		vTaskDelay(200);
	}
}

/* Using the flowrate from the flow task, spawn a new car in random intervals depending on current flow rate */
void TrafficCreatorTask(void *pvParameters)
{
	uint16_t car_value = 0, flowrate = 4;

	while (1)
	{
		printf("Start Traffic Creator Task\n");

		// Get Flow Semaphore
		if (xSemaphoreTake(xMutexFlow, (TickType_t)10) == pdTRUE)
		{
			xQueueReceive(xQueueFlowRate, &flowrate, 0);
			xSemaphoreGive(xMutexFlow);
		}
		else
			printf("xMutexFlow already held\n");

		// 
		float randomNum = 100.0 / (8.0 - flowrate);
		car_value = (rand() % 100) < randomNum;

		// Get Cars Semaphore
		if (xSemaphoreTake(xMutexCars, (TickType_t)10) == pdTRUE)
		{
			xQueueOverwrite(xQueueCarValue, &car_value);
			xSemaphoreGive(xMutexCars);
		}
		else
			printf("xMutexCars already held\n");

		vTaskDelay(800);
	}
}

/* Display necessary lights on road and traffic light */
void TrafficDisplayTask(void *pvParameters)
{
	uint16_t car_value = 0, light_colour = 1;
	uint16_t pre_light_traffic[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t mid_light_traffic[3] = {0, 0, 0};
	uint16_t new_pre_light_traffic[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	while (1)
	{
		// Get Cars Semaphore
		if (xSemaphoreTake(xMutexCars, (TickType_t)10) == pdTRUE)
		{
			xQueueReceive(xQueueCarValue, &car_value, 0);
			xSemaphoreGive(xMutexCars);
		}
		else
			printf("xMutexCars already held\n");

		// Get Light Semaphore
		if (xSemaphoreTake(xMutexLight, (TickType_t)0) == pdTRUE)
		{
			xQueuePeek(xQueueLightColour, &light_colour, 0);
			xSemaphoreGive(xMutexLight);
		}
		else
			printf("xMutexLight already held\n");

		// If light's green, spawn new car and move others
		if (light_colour)
		{
			// Spawn new car and move other cars
			ShiftRegisterValuePreLight(car_value);
			ShiftRegisterValueMidLight(pre_light_traffic[7]);
			ShiftRegisterValuePostLight(mid_light_traffic[2]);

			// Add new car to array
			new_pre_light_traffic[0] = car_value;

			// Shift and merge pre-light traffic into new array
			for (uint16_t i = 1; i < 8; i++)
				new_pre_light_traffic[i] = pre_light_traffic[i - 1];

			// Shift and merge mid-light traffic into new array
			for (uint16_t i = 2; i != 0; i--)
				mid_light_traffic[i] = mid_light_traffic[i - 1];

			// Add car entering intersection
			mid_light_traffic[0] = pre_light_traffic[7];
		}

		// Light is red or yellow
		else
		{
			uint16_t encounteredzero = 0;

			// Find empty spots on road
			for (uint16_t i = 7; i != 0; i--)
			{
				if (pre_light_traffic[i] == 0)
				{
					encounteredzero = 1;
					new_pre_light_traffic[0] = car_value;
				}

				// Shift cars if open spot
				if (encounteredzero == 1)
					new_pre_light_traffic[i] = pre_light_traffic[i - 1];
				else
					new_pre_light_traffic[i] = pre_light_traffic[i];
			}

			// Shift cars mid light, and shift mid array
			ShiftRegisterValueMidLight(0);

			for (uint16_t i = 1; i != 3; i++)
				mid_light_traffic[i] = mid_light_traffic[i - 1];

			// Shift cars post light, and update mid array [0]
			ShiftRegisterValuePostLight(mid_light_traffic[2]);
			mid_light_traffic[0] = 0;

			// Update cars pre-light on red
			for (int16_t i = 8; i > 0; i--) {
				ShiftRegisterValuePreLight(new_pre_light_traffic[i-1]);
			}

		}

		// Update current active array
		for (uint16_t i = 0; i != 8; i++)
			pre_light_traffic[i] = new_pre_light_traffic[i];

		vTaskDelay(800);
	}
}

/* Update Register before intersection */
void ShiftRegisterValuePreLight(uint16_t value)
{
	if (value)
		GPIO_SetBits(SHIFT_REG_1_PORT, SHIFT_REG_1_PIN);
	else
		GPIO_ResetBits(SHIFT_REG_1_PORT, SHIFT_REG_1_PIN);

	// Reset
	GPIO_SetBits(SHIFT_REG_1_PORT, SHIFT_REG_CLK_1_PIN);
	GPIO_ResetBits(SHIFT_REG_1_PORT, SHIFT_REG_CLK_1_PIN);
}

/* Update Register in intersection */
void ShiftRegisterValueMidLight(uint16_t value)
{
	if (value)
		GPIO_SetBits(SHIFT_REG_2_PORT, SHIFT_REG_2_PIN);
	else
		GPIO_ResetBits(SHIFT_REG_2_PORT, SHIFT_REG_2_PIN);

	// Reset
	GPIO_SetBits(SHIFT_REG_1_PORT, SHIFT_REG_CLK_1_PIN);
	GPIO_ResetBits(SHIFT_REG_1_PORT, SHIFT_REG_CLK_1_PIN);
}

/* Update Register after intersection */
void ShiftRegisterValuePostLight(uint16_t value)
{
	if (value)
		GPIO_SetBits(SHIFT_REG_3_PORT, SHIFT_REG_3_PIN);
	else
		GPIO_ResetBits(SHIFT_REG_3_PORT, SHIFT_REG_3_PIN);

	// Reset
	GPIO_SetBits(SHIFT_REG_1_PORT, SHIFT_REG_CLK_1_PIN);
	GPIO_ResetBits(SHIFT_REG_1_PORT, SHIFT_REG_CLK_1_PIN);
}
