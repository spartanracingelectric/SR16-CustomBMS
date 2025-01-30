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
#define CAN_TX_QUEUE_SIZE 50  //size of queue
typedef struct {
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t data[8];
} CANMessage;

/* USER CODE END Private defines */

void MX_CAN1_Init(void);

/* USER CODE BEGIN Prototypes */
HAL_StatusTypeDef CAN_Start();
HAL_StatusTypeDef CAN_Activate();
HAL_StatusTypeDef CAN_Send(struct CANMessage *ptr);
void CAN_SettingsInit(struct CANMessage *ptr);
void Set_CAN_Id(struct CANMessage *ptr, uint32_t id);

void CAN_Send_Voltage(struct CANMessage *ptr, uint16_t *read_volt);
void CAN_Send_Temperature(struct CANMessage *ptr, uint16_t *read_temp);
void CAN_Send_Cell_Summary(struct CANMessage *ptr, struct batteryModule *batt);
void CAN_Send_Safety_Checker(struct CANMessage *ptr, struct batteryModule *batt, uint8_t* faults, uint8_t* warnings, uint8_t *states);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

