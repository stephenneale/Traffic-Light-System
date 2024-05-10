/*
 * Traffic.h
 *
 */

#ifndef TRAFFICCONTROL_H_
#define TRAFFICCONTROL_H_

#include <Config.h>

void TrafficDisplayTask( void *pvParameters );
void TrafficFlowAdjustmentTask( void *pvParameters );
void TrafficCreatorTask( void *pvParameters );

// Helper function declarations
void ShiftRegisterValuePreLight( uint16_t value );
void ShiftRegisterValuePostLight( uint16_t value );
void ShiftRegisterValueMidLight( uint16_t value );
#endif /* TRAFFICCONTROL_H_ */

