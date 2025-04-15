/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE BEGIN Private defines */
#define NUM_DEVICES				8	//1 slave board
#define NUM_CELL_SERIES_GROUP	12	//1 slave board
#define NUM_CELLS				NUM_DEVICES*NUM_CELL_SERIES_GROUP	//multiple slave board
#define NUM_THERM_PER_MOD		12
#define NUM_THERM_TOTAL			NUM_DEVICES*NUM_THERM_PER_MOD
#define NUM_AUX_GROUP			6
#define NUM_AUXES				NUM_DEVICES*NUM_AUX_GROUP
#define CYCLETIME_CAP			60  //60ms update delay
#define CAN_RECONNECTION_CHECK	500 //check can connection every 500ms
#define LED_HEARTBEAT_DELAY_MS	50  //50ms update delay
#define BALANCE 				0 	//FALSE
#define MAX_CELL_CAPACITY 		3000
#define MAX_BATTERY_CAPACITY 	NUM_DEVICES* MAX_CELL_CAPACITY

#define SHUNT_SIGNAL_Pin GPIO_PIN_3
#define SHUNT_SIGNAL_GPIO_Port GPIOC
#define LTC_nCS_Pin GPIO_PIN_4
#define LTC_nCS_GPIO_Port GPIOA
#define MCU_ADC_VSENSE_Pin GPIO_PIN_5
#define MCU_ADC_VSENSE_GPIO_Port GPIOC
#define MCU_SHUTDOWN_SIGNAL_Pin GPIO_PIN_1
#define MCU_SHUTDOWN_SIGNAL_GPIO_Port GPIOB
#define MCU_HEARTBEAT_LED_Pin GPIO_PIN_6
#define MCU_HEARTBEAT_LED_GPIO_Port GPIOC
/* USER CODE END Private defines */

typedef struct batteryModule {
	uint16_t cell_volt[NUM_CELLS];
	uint16_t cell_temp[NUM_THERM_TOTAL];
	uint16_t average_volt[NUM_DEVICES];
	uint16_t average_temp[NUM_DEVICES];
	uint16_t pressure[NUM_DEVICES];
	uint16_t humidity[NUM_DEVICES];
	uint16_t atmos_temp[NUM_DEVICES];
	uint16_t cell_volt_lowest;
	uint16_t cell_volt_highest;
	uint16_t cell_difference;
	uint16_t cell_temp_lowest;
	uint16_t cell_temp_highest;
	uint16_t sum_pack_voltage;
	uint16_t hvsens_pack_voltage;
	uint16_t read_auxreg[NUM_AUXES];
	uint16_t balance_status[NUM_DEVICES];
    uint16_t soc;
    uint32_t current;
    uint16_t dew_point[NUM_DEVICES];
} batteryModule;

typedef struct CANMessage{
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    uint8_t voltageBuffer[8];
    uint8_t thermistorBuffer[8];
    uint8_t summaryBuffer[8];
    uint8_t safetyBuffer[8];
    uint8_t socBuffer[8];
    uint8_t balanceStatus[8];
} CANMessage;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
