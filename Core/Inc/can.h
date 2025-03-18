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

/* USER CODE END Private defines */

void MX_CAN1_Init(void);

/* USER CODE BEGIN Prototypes */

HAL_StatusTypeDef CAN_Start();
HAL_StatusTypeDef CAN_Activate();
HAL_StatusTypeDef CAN_Send(CANMessage *ptr);
void CAN_SettingsInit(CANMessage *ptr);
void Set_CAN_Id(CANMessage *ptr, uint32_t id);

void CAN_Send_Voltage(CANMessage *ptr, uint16_t *read_volt);
void CAN_Send_Temperature(CANMessage *ptr, uint16_t *read_temp);
void CAN_Send_Cell_Summary(CANMessage *ptr, struct batteryModule *batt);
void CAN_Send_Safety_Checker(CANMessage *ptr, struct batteryModule *batt, uint8_t* faults, uint8_t* warnings);
void CAN_Send_SOC(CANMessage *ptr, batteryModule *batt, uint16_t max_capacity);
void CAN_Send_Balance_Status(struct CANMessage *ptr, uint16_t *balance_status);
void CAN_Send_Sensor(struct CANMessage *ptr, batteryModule *batt);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

