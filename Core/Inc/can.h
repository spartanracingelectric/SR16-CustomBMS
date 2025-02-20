/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */


/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN Private defines */
#define CAN_ID_VOLTAGE 				1584
#define CAN_ID_THERMISTOR 			1664
#define CAN_ID_SUMMARY				1570
#define CAN_ID_SAFETY 				1536
#define CAN_ID_SOC 					1537
#define CAN_BYTE_NUM				8
#define CAN_MESSAGE_NUM_VOLTAGE 	NUM_CELLS * 2 / CAN_BYTE_NUM
#define CAN_MESSAGE_NUM_THERMISTOR 	NUM_THERM_TOTAL * 2 / CAN_BYTE_NUM
//#define CAN_ID_Balance 		0x630
/* USER CODE END Private defines */

void MX_CAN1_Init(void);

/* USER CODE BEGIN Prototypes */

HAL_StatusTypeDef CAN_Start();
HAL_StatusTypeDef CAN_Activate();
HAL_StatusTypeDef CAN_Send(CANMessage *ptr);
void CAN_SettingsInit(CANMessage *ptr);
void Set_CAN_Id(CANMessage *ptr, uint32_t id);

void Pack_CAN_Message_Voltage(CANMessage *ptr, uint16_t *read_volt);

void Pack_CAN_Message_Temeperature(CANMessage *ptr, uint16_t *read_temp);

void Pack_CAN_Message_Cell_Summary(CANMessage *ptr, batteryModule *batt);

void Pack_CAN_Message_Safety_Checker(CANMessage *ptr, batteryModule *batt, uint8_t *faults, uint8_t *warnings, uint8_t *states);
void Pack_CAN_Message_SoC(CANMessage *ptr, batteryModule *batt,
        uint16_t max_capacity);

void CAN_Send_Voltage(CANMessage *ptr, uint16_t *read_volt);
void CAN_Send_Temperature(CANMessage *ptr, uint16_t *read_temp);
void CAN_Send_Cell_Summary(CANMessage *ptr, batteryModule *batt);
void CAN_Send_Safety_Checker(CANMessage *ptr, batteryModule *batt, uint8_t* faults, uint8_t* warnings, uint8_t *states);
void CAN_Send_SOC(CANMessage *ptr, batteryModule *batt, uint16_t max_capacity);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

