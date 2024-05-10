/*
 * TrafficLight.h
 *
 */

#ifndef TRAFFICLIGHT_H_
#define TRAFFICLIGHT_H_

#include <Config.h>

void Green_LED_Controller_Callback( xTimerHandle xTimer );
void Yellow_LED_Controller_Callback( xTimerHandle xTimer );
void Red_LED_Controller_Callback( xTimerHandle xTimer );

void TrafficLightTask( void *pvParameters );

xTimerHandle xRedLightSoftwareTimer;
xTimerHandle xYellowLightSoftwareTimer;
xTimerHandle xGreenLightSoftwareTimer;


#endif /* TRAFFICLIGHT_H_ */
