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

/* USER CODE BEGIN Includes */
#include "accumulator.h"
#include "main.h"

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN1_Init(void);

/* USER CODE BEGIN Prototypes */
typedef struct CANMessage {
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    uint8_t data[8];
} CANMessage;

HAL_StatusTypeDef CAN_Start();
HAL_StatusTypeDef CAN_Activate();
HAL_StatusTypeDef CAN_Send(CANMessage *ptr);

void CAN_initSettings(CANMessage *ptr);
void CAN_sendVoltages(CANMessage *ptr, uint16_t *read_volt);
void CAN_sendTemperatures(CANMessage *ptr, uint16_t *read_temp);
void CAN_sendCellSummary(CANMessage *ptr, Accumulator *batt);
void CAN_sendSafetyChecker(CANMessage *ptr, Accumulator *batt, uint8_t *faults,
                           uint8_t *warnings, uint8_t *states);
void CAN_sendStateOfCharge(CANMessage *ptr, Accumulator *batt,
                           uint16_t max_capacity);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */
