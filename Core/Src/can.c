/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

#include "accumulator.h"
#include "error.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

void CAN_setId(CANMessage *ptr, uint32_t id);

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void) {
    /* USER CODE BEGIN CAN1_Init 0 */

    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 9;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_3TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN1_Init 2 */

    /* USER CODE END CAN1_Init 2 */
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspInit 0 */

        /* USER CODE END CAN1_MspInit 0 */
        /* CAN1 clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**CAN1 GPIO Configuration
        PB8     ------> CAN1_RX
        PB9     ------> CAN1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        __HAL_AFIO_REMAP_CAN1_2();

        /* CAN1 interrupt Init */
        HAL_NVIC_SetPriority(CAN1_TX_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
        HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
        /* USER CODE BEGIN CAN1_MspInit 1 */

        /* USER CODE END CAN1_MspInit 1 */
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle) {
    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspDeInit 0 */

        /* USER CODE END CAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN1 GPIO Configuration
        PB8     ------> CAN1_RX
        PB9     ------> CAN1_TX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);

        /* CAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
        /* USER CODE BEGIN CAN1_MspDeInit 1 */

        /* USER CODE END CAN1_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

// uint8_t CAN_TX_HALT = 1; //halt frag to send it to mailbox

HAL_StatusTypeDef CAN_Start() { return HAL_CAN_Start(&hcan1); }

HAL_StatusTypeDef CAN_Activate() {
    return HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

HAL_StatusTypeDef CAN_Send(CANMessage *ptr) {
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
    }
    return HAL_CAN_AddTxMessage(&hcan1, &ptr->TxHeader, (uint8_t *)ptr->data,
                                &ptr->TxMailbox);
}

// void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
//	CAN_TX_HALT = 0;
// }
// void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
//	CAN_TX_HALT = 0;
// }
// void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
//	CAN_TX_HALT = 0;
// }

void CAN_initSettings(CANMessage *ptr) {
    CAN_Start();
    CAN_Activate();
    ptr->TxHeader.IDE = CAN_ID_STD;
    ptr->TxHeader.StdId = 0x00;
    ptr->TxHeader.RTR = CAN_RTR_DATA;
    ptr->TxHeader.DLC = 8;
}

void CAN_setId(CANMessage *ptr, uint32_t id) { ptr->TxHeader.StdId = id; }

void CAN_sendVoltages(CANMessage *ptr, uint16_t *read_volt) {
    uint16_t CAN_ID = 0x630;
    CAN_setId(ptr, CAN_ID);
    for (int i = 0; i < NUM_CELLS; i++) {
        if (i % 4 == 0) {
            uint8_t temp_volt = i;
            ptr->data[0] = read_volt[temp_volt];
            ptr->data[1] = read_volt[temp_volt] >> 8;
            temp_volt += 1;
            ptr->data[2] = read_volt[temp_volt];
            ptr->data[3] = read_volt[temp_volt] >> 8;
            temp_volt += 1;
            ptr->data[4] = read_volt[temp_volt];
            ptr->data[5] = read_volt[temp_volt] >> 8;
            temp_volt += 1;
            ptr->data[6] = read_volt[temp_volt];
            ptr->data[7] = read_volt[temp_volt] >> 8;
        }
        if (i > 0 && i % 4 == 0) {
            CAN_ID = CAN_ID + 0x01;
            CAN_setId(ptr, CAN_ID);
        }
        CAN_Send(ptr);
        //		printf("voltage\n");
    }
}

void CAN_sendTemperatures(CANMessage *ptr, uint16_t *read_temp) {
    uint16_t CAN_ID = 0x680;
    CAN_setId(ptr, CAN_ID);
    for (uint8_t i = 0; i < NUM_THERM_TOTAL; i++) {
        if (i % 4 == 0) {
            uint8_t temp_volt = i;
            ptr->data[0] = read_temp[temp_volt];
            ptr->data[1] = read_temp[temp_volt] >> 8;
            temp_volt += 1;
            ptr->data[2] = read_temp[temp_volt];
            ptr->data[3] = read_temp[temp_volt] >> 8;
            temp_volt += 1;
            ptr->data[4] = read_temp[temp_volt];
            ptr->data[5] = read_temp[temp_volt] >> 8;
            temp_volt += 1;
            ptr->data[6] = read_temp[temp_volt];
            ptr->data[7] = read_temp[temp_volt] >> 8;
        }
        if (i > 0 && i % 4 == 0) {
            CAN_ID = CAN_ID + 0x01;
            CAN_setId(ptr, CAN_ID);
        }
        CAN_Send(ptr);
        //	printf("Temperature\n");
    }
}

void CAN_sendCellSummary(CANMessage *ptr, Accumulator *batt) {
    uint16_t CAN_ID = 0x622;
    CAN_setId(ptr, CAN_ID);

    ptr->data[0] = batt->cell_volt_highest;
    ptr->data[1] = (batt->cell_volt_highest) >> 8;
    ptr->data[2] = batt->cell_volt_lowest;
    ptr->data[3] = (batt->cell_volt_lowest) >> 8;
    ptr->data[4] = batt->cell_temp_highest;
    ptr->data[5] = (batt->cell_temp_highest) >> 8;
    ptr->data[6] = batt->cell_temp_lowest;
    ptr->data[7] = (batt->cell_temp_lowest) >> 8;
    CAN_Send(ptr);
    //	printf("Summary\n");
}

void CAN_sendSafetyChecker(CANMessage *ptr, Accumulator *batt, uint8_t *faults,
                           uint8_t *warnings, uint8_t *states) {
    uint16_t CAN_ID = 0x600;
    CAN_setId(ptr, CAN_ID);
    ptr->data[0] = *faults;
    ptr->data[1] = *warnings;
    ptr->data[2] = *states;
    ptr->data[3] = batt->pack_voltage;
    ptr->data[4] = (batt->pack_voltage) >> 8;
    ptr->data[5] = (batt->pack_voltage) >> 16;
    ptr->data[6] = (batt->pack_voltage) >> 24;
    CAN_Send(ptr);
    //	printf("Faults\n");
}

void CAN_sendStateOfCharge(CANMessage *ptr, Accumulator *batt,
                           uint16_t max_capacity) {
    uint16_t CAN_ID = 0x601;
    CAN_setId(ptr, CAN_ID);

    ptr->data[0] = batt->soc;
    ptr->data[1] = batt->soc >> 8;

    uint8_t percent = (uint8_t)(batt->soc * 100 / max_capacity);
    ptr->data[2] = percent;
    ptr->data[3] = batt->current;
    ptr->data[4] = batt->current >> 8;
    ptr->data[5] = batt->current >> 16;
    ptr->data[6] = batt->current >> 24;
    CAN_Send(ptr);
}
/* USER CODE END 1 */
