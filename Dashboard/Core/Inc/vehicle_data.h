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
	uint8_t Speed;
	uint8_t Recommend_Speed;
	uint8_t MPPT_Power;
	uint8_t Motor_Power;
	uint8_t Battery_SOC;
	bool Battery_State;
	bool Signal_State;
} VehicleState_t;

typedef struct
{
	uint8_t Info_1;
	uint8_t Info_2;
	uint16_t Info_3;
	uint8_t Info_4;
	uint8_t Time_Hour;
	uint8_t Time_Min;
	bool Test_Value1;
	bool Test_Value2;
	bool Test_Value3;
} SystemInfo_t;

/* Light 관련 구조체 */
typedef struct
{
	bool Left_Signal;
	bool Right_Signal;
	bool Hazard_Signal;
} LightState_t;

/* ACC 관련 구조체 */
typedef struct
{
	bool Acc_state;
	uint8_t Acc_speed; // 기본값 70
} AccState_t;

extern VehicleState_t DrivingData;
extern SystemInfo_t InfoData;
extern LightState_t LightData;
extern AccState_t AccData;
extern uint8_t Fault_Status[100];
extern bool Fault_Data_Changed;

#endif /* INC_VEHICLE_DATA_H_ */
