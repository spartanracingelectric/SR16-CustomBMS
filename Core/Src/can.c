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
#include "usart.h"
#include "stdio.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

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
uint8_t  thermistor8BitsPrevious[NUM_THERM_TOTAL];
// uint8_t CAN_TX_HALT = 1; //halt frag to send it to mailbox

HAL_StatusTypeDef CAN_Start() { return HAL_CAN_Start(&hcan1); }

HAL_StatusTypeDef CAN_Activate() {
    return HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

HAL_StatusTypeDef CAN_Send(CANMessage *ptr) {
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
    }

    uint8_t *dataPtr = NULL;

       if(ptr->TxHeader.StdId >= CAN_ID_VOLTAGE &&  ptr->TxHeader.StdId < CAN_ID_VOLTAGE + (NUM_CELLS * 2 / CAN_BYTE_NUM)) {//(NUM_CELLS * 2 / CAN_BYTE_NUM is just a number of can message
    	   dataPtr = (uint8_t *)ptr->voltageBuffer;
       }
       if(ptr->TxHeader.StdId >= CAN_ID_THERMISTOR &&  ptr->TxHeader.StdId < CAN_ID_THERMISTOR + (NUM_THERM_TOTAL / CAN_BYTE_NUM + 3)) {//(NUM_THERM_TOTAL / CAN_BYTE_NUM + 3) is a number of the message, 3 is number of sensors
    	   dataPtr = (uint8_t *)ptr->thermistorBuffer;
       }
       if(ptr->TxHeader.StdId == CAN_ID_SUMMARY) {
    	   dataPtr = (uint8_t *)ptr->summaryBuffer;
       }
       if(ptr->TxHeader.StdId == CAN_ID_SAFETY) {
    	   dataPtr = (uint8_t *)ptr->safetyBuffer;
       }
       if(ptr->TxHeader.StdId == CAN_ID_SOC) {
    	   dataPtr = (uint8_t *)ptr->socBuffer;
       }

    return HAL_CAN_AddTxMessage(&hcan1, &ptr->TxHeader, dataPtr, &ptr->TxMailbox);
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

void CAN_SettingsInit(CANMessage *buffers) {
    CAN_Start();
    CAN_Activate();
    buffers->TxHeader.IDE = CAN_ID_STD;
    buffers->TxHeader.StdId = 0x00;
    buffers->TxHeader.RTR = CAN_RTR_DATA;
    buffers->TxHeader.DLC = 8;
}

void Set_CAN_Id(CANMessage *ptr, uint32_t id) { ptr->TxHeader.StdId = id; }

void Send_CAN_Message_Voltage(CANMessage *buffer, uint16_t *read_volt){
	uint32_t CAN_ID = (uint32_t)CAN_ID_VOLTAGE;
	for (int i = 0; i < NUM_CELLS; i += 4) {  //pack every 4 cell group in 1 CAN message
		buffer->voltageBuffer[0] =  read_volt[i] & 0xFF; 			//To ensure the data type is uint8_t, use & 0xFF
		buffer->voltageBuffer[1] = (read_volt[i] >> 8) & 0xFF;
		buffer->voltageBuffer[2] =  read_volt[i + 1] & 0xFF;
		buffer->voltageBuffer[3] = (read_volt[i + 1] >> 8) & 0xFF;
		buffer->voltageBuffer[4] =  read_volt[i + 2] & 0xFF;
		buffer->voltageBuffer[5] = (read_volt[i + 2] >> 8) & 0xFF;
		buffer->voltageBuffer[6] =  read_volt[i + 3] & 0xFF;
		buffer->voltageBuffer[7] = (read_volt[i + 3] >> 8) & 0xFF;

		Set_CAN_Id(buffer, CAN_ID);
		CAN_Send(buffer);
//		printf("CAN_ID: 0x%X\n", CAN_ID);
		CAN_ID++;

	}
}

void Send_CAN_Message_Temperature(CANMessage *buffer, uint16_t *read_temp) {
    uint32_t CAN_ID = (uint32_t)CAN_ID_THERMISTOR;

    for (int i = 0; i < NUM_THERM_TOTAL + 24; i += 16) {
        Set_CAN_Id(buffer, CAN_ID);

		buffer->thermistorBuffer[0] = (uint8_t)(read_temp[  i  ] & 0xFF);
		buffer->thermistorBuffer[1] = (uint8_t)(read_temp[i + 1] & 0xFF);
		buffer->thermistorBuffer[2] = (uint8_t)(read_temp[i + 2] & 0xFF);
		buffer->thermistorBuffer[3] = (uint8_t)(read_temp[i + 3] & 0xFF);
		buffer->thermistorBuffer[4] = (uint8_t)(read_temp[i + 4] & 0xFF);
		buffer->thermistorBuffer[5] = (uint8_t)(read_temp[i + 5] & 0xFF);
		buffer->thermistorBuffer[6] = (uint8_t)(read_temp[i + 6] & 0xFF);
		buffer->thermistorBuffer[7] = (uint8_t)(read_temp[i + 7] & 0xFF);

		printf("temp1 in 8 bits:%d\n", buffer->thermistorBuffer[0]);
		printf("temp2 in 8 bits:%d\n", buffer->thermistorBuffer[1]);
		printf("temp3 in 8 bits:%d\n", buffer->thermistorBuffer[2]);
		printf("temp4 in 8 bits:%d\n", buffer->thermistorBuffer[3]);
		printf("temp5 in 8 bits:%d\n", buffer->thermistorBuffer[4]);
		printf("temp6 in 8 bits:%d\n", buffer->thermistorBuffer[5]);
		printf("temp7 in 8 bits:%d\n", buffer->thermistorBuffer[6]);
		printf("temp8 in 8 bits:%d\n", buffer->thermistorBuffer[7]);

		CAN_Send(buffer);
		CAN_ID++;

		Set_CAN_Id(buffer, CAN_ID);

		buffer->thermistorBuffer[0] = (uint8_t)(read_temp[i + 8] & 0xFF);
		buffer->thermistorBuffer[1] = (uint8_t)(read_temp[i + 9] & 0xFF);
		buffer->thermistorBuffer[2] = (uint8_t)(read_temp[i + 10] & 0xFF);
		buffer->thermistorBuffer[3] = (uint8_t)(read_temp[i + 11] & 0xFF);
		buffer->thermistorBuffer[4] =  0xFF;
		buffer->thermistorBuffer[5] =  0xFF;
		buffer->thermistorBuffer[6] =  0xFF;
		buffer->thermistorBuffer[7] =  0xFF;

		printf("temp9 in 8 bits:%d\n", buffer->thermistorBuffer[0]);
		printf("temp10 in 8 bits:%d\n", buffer->thermistorBuffer[1]);
		printf("temp11 in 8 bits:%d\n", buffer->thermistorBuffer[2]);
		printf("temp12 in 8 bits:%d\n", buffer->thermistorBuffer[3]);

		CAN_Send(buffer);
		CAN_ID++;
//      printf("sending CAN");
		}
//	for(int i = 0; i < 96; i ++){
//		uint8_t eightbit = (uint8_t)(read_temp[i]);
//		uint16_t sixteenbit = read_temp[i];
//		printf("temp[%d] in 8 bits:%d\n", i, eightbit);
//		printf("temp[%d] in 16 bits:%d\n", i, sixteenbit);
//	}
}


void Send_CAN_Message_Cell_Summary(CANMessage *buffer, batteryModule *batt){
	uint32_t CAN_ID = (uint32_t)CAN_ID_SUMMARY;
	buffer->summaryBuffer[0] = batt->cell_volt_highest & 0xFF;
	buffer->summaryBuffer[1] = (batt->cell_volt_highest >> 8) & 0xFF;
	buffer->summaryBuffer[2] = batt->cell_volt_lowest & 0xFF;
	buffer->summaryBuffer[3] = (batt->cell_volt_lowest >> 8) & 0xFF;
	buffer->summaryBuffer[4] = (uint8_t)batt->cell_temp_highest & 0xFF;
	buffer->summaryBuffer[5] = (uint8_t)batt->cell_temp_lowest & 0xFF;
//	buffer->summaryBuffer[6] =
//	buffer->summaryBuffer[7] =
	Set_CAN_Id(buffer, CAN_ID);
	CAN_Send(buffer);
}

void Send_CAN_Message_Safety_Checker(CANMessage *buffer, batteryModule *batt, uint8_t *faults, uint8_t *warnings, uint8_t *states){
	uint32_t CAN_ID = (uint32_t)CAN_ID_SAFETY;
	buffer->safetyBuffer[0] = *faults;
	buffer->safetyBuffer[1] = *warnings;
	buffer->safetyBuffer[2] = *states;
	buffer->safetyBuffer[3] = batt->pack_voltage & 0xFF;
	buffer->safetyBuffer[4] = (batt->pack_voltage >> 8) & 0xFF;
//	buffer->safetyBuffer[5] = (batt->pack_voltage >> 16) & 0xFF;
//	buffer->safetyBuffer[6] = (batt->pack_voltage >> 24) & 0xFF;
	Set_CAN_Id(buffer, CAN_ID);
	CAN_Send(buffer);
}

void Send_CAN_Message_SoC(CANMessage *buffer, batteryModule *batt,
    uint16_t max_capacity){
	uint32_t CAN_ID = (uint32_t)CAN_ID_SOC;
	uint8_t percent = (uint8_t)(batt->soc * 100 / max_capacity);
	buffer->socBuffer[0] = batt->soc;
	buffer->socBuffer[1] = batt->soc >> 8;
    buffer->socBuffer[2] = percent;
    buffer->socBuffer[3] = batt->current & 0xFF;
    buffer->socBuffer[4] = (batt->current >> 8) & 0xFF;
    buffer->socBuffer[5] = (batt->current >> 16) & 0xFF;
    buffer->socBuffer[6] = (batt->current >> 24)& 0xFF;
	Set_CAN_Id(buffer, CAN_ID);
	CAN_Send(buffer);
}
/* USER CODE END 1 */
