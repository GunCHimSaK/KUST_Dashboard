/*
 * vehicle_data.h
 *
 *  Created on: May 11, 2026
 *      Author: Han Gyujin
 */

#ifndef VEHICLE_DATA_H_
#define VEHICLE_DATA_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t  Speed;
    uint8_t  Recommend_Speed;
    uint8_t  MPPT_Power;
    uint8_t  Motor_Power;
    uint8_t  Battery_SOC;
    bool     Battery_State;
    bool     Signal_State;
} VehicleState_t;

typedef struct
{
    uint8_t  Info_1;
    uint8_t  Info_2;
    uint16_t  Info_3;
    uint8_t  Info_4;
    uint8_t  Time_Hour;
    uint8_t  Time_Min;
} SystemInfo_t;

extern VehicleState_t  DrivingData;
extern SystemInfo_t    InfoData;
extern uint8_t Fault_Status[100];

#endif /* INC_VEHICLE_DATA_H_ */
