/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
//#include "main.h"
#include "module.h"
#include "can.h"
#include "safety.h"
#include "adc.h"
#include "spi.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
typedef struct {
    uint8_t tempIndex;            // サーミスタインデックス
    uint16_t readTemp[NUM_DEVICES]; // デ�?イス�?��?��?�温度データ
    uint16_t readAuxReg[NUM_DEVICES * NUM_AUX_GROUP]; // 補助レジスタデータ
} TempData_t;
struct batteryModule modPackInfo;
struct CANMessage msg;
uint8_t safetyFaults = 0;
uint8_t safetyWarnings = 0;
uint8_t safetyStates = 0;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for HartBeatLED */
osThreadId_t HartBeatLEDHandle;
const osThreadAttr_t HartBeatLED_attributes = {
  .name = "HartBeatLED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ReadVolt */
osThreadId_t ReadVoltHandle;
const osThreadAttr_t ReadVolt_attributes = {
  .name = "ReadVolt",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ReadTemp */
osThreadId_t ReadTempHandle;
const osThreadAttr_t ReadTemp_attributes = {
  .name = "ReadTemp",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CellSummaryVolt */
osThreadId_t CellSummaryVoltHandle;
const osThreadAttr_t CellSummaryVolt_attributes = {
  .name = "CellSummaryVolt",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CellSummaryTemp */
osThreadId_t CellSummaryTempHandle;
const osThreadAttr_t CellSummaryTemp_attributes = {
  .name = "CellSummaryTemp",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for StartBalance */
osThreadId_t StartBalanceHandle;
const osThreadAttr_t StartBalance_attributes = {
  .name = "StartBalance",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANVolt */
osThreadId_t CANVoltHandle;
const osThreadAttr_t CANVolt_attributes = {
  .name = "CANVolt",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANTemp */
osThreadId_t CANTempHandle;
const osThreadAttr_t CANTemp_attributes = {
  .name = "CANTemp",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANSummary */
osThreadId_t CANSummaryHandle;
const osThreadAttr_t CANSummary_attributes = {
  .name = "CANSummary",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANFault */
osThreadId_t CANFaultHandle;
const osThreadAttr_t CANFault_attributes = {
  .name = "CANFault",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartHartBeatLED(void *argument);
void StartReadVolt(void *argument);
void StartReadTemp(void *argument);
void StartCellSummaryVoltage(void *argument);
void StartCellSummaryTemperature(void *argument);
void StartStartBalance(void *argument);
void StartCANVolt(void *argument);
void StartCANTemp(void *argument);
void StartCANSummary(void *argument);
void StartCANFault(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of HartBeatLED */
  HartBeatLEDHandle = osThreadNew(StartHartBeatLED, NULL, &HartBeatLED_attributes);

  /* creation of ReadVolt */
  ReadVoltHandle = osThreadNew(StartReadVolt, NULL, &ReadVolt_attributes);

  /* creation of ReadTemp */
  ReadTempHandle = osThreadNew(StartReadTemp, NULL, &ReadTemp_attributes);

  /* creation of CellSummaryVolt */
  CellSummaryVoltHandle = osThreadNew(StartCellSummaryVoltage, NULL, &CellSummaryVolt_attributes);

  /* creation of CellSummaryTemp */
  CellSummaryTempHandle = osThreadNew(StartCellSummaryTemperature, NULL, &CellSummaryTemp_attributes);

  /* creation of StartBalance */
  StartBalanceHandle = osThreadNew(StartStartBalance, NULL, &StartBalance_attributes);

  /* creation of CANVolt */
  CANVoltHandle = osThreadNew(StartCANVolt, NULL, &CANVolt_attributes);

  /* creation of CANTemp */
  CANTempHandle = osThreadNew(StartCANTemp, NULL, &CANTemp_attributes);

  /* creation of CANSummary */
  CANSummaryHandle = osThreadNew(StartCANSummary, NULL, &CANSummary_attributes);

  /* creation of CANFault */
  CANFaultHandle = osThreadNew(StartCANFault, NULL, &CANFault_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartHartBeatLED */
/**
  * @brief  Function implementing the HartBeatLED thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartHartBeatLED */
void StartHartBeatLED(void *argument)
{
  /* USER CODE BEGIN StartHartBeatLED */
  /* Infinite loop */
  for(;;)
  {
	  GpioFixedToggle(&tp_led_heartbeat, 1000); // Toggle heat beat LED every 1 sec
	  osDelay(1000);

  }
  /* USER CODE END StartHartBeatLED */
}

/* USER CODE BEGIN Header_StartReadVolt */
/**
* @brief Function implementing the ReadVolt thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReadVolt */
void StartReadVolt(void *argument)
{
  /* USER CODE BEGIN StartReadVolt */
  /* Infinite loop */
  for(;;)
  {
	  //	  if (osMutexAcquire(VoltageQueueMutexHandle, osWaitForever) == osOK) {
	  		  Read_Volt(modPackInfo.cell_volt);

//	  		  uint16_t temp_voltages[NUM_CELLS];
//	  		  memcpy(temp_voltages, modPackInfo.cell_volt, sizeof(temp_voltages));
//
//	  		  if (xQueueSend(VoltQueueHandle, temp_voltages, 0) != pdPASS) {
//	  		      printf("Failed to send voltage array to voltage queue.\n");
//	  		  }

	  	  // ミューテックス解放
	  //	  osMutexRelease(VoltageQueueMutexHandle);
	  //	  }
	  //	  else{
	  //		  printf("Failed to acquire mutex for voltage queue!\n");
	  //	  }

	  	  osDelay(205);
  }
  /* USER CODE END StartReadVolt */
}

/* USER CODE BEGIN Header_StartReadTemp */
/**
* @brief Function implementing the ReadTemp thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReadTemp */
void StartReadTemp(void *argument)
{
  /* USER CODE BEGIN StartReadTemp */
  /* Infinite loop */
  for(;;)
  {
//	  if (osMutexAcquire(TempQueueMutexHandle, osWaitForever) == osOK) {
	  for (uint8_t i = tempindex; i < indexpause; i++) {
		  Wakeup_Idle();
		  Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);
		  osDelay(2);
	  }
	  if (indexpause == 8) {
		  Wakeup_Idle();
		  LTC_WRCOMM(NUM_DEVICES, BMS_MUX_PAUSE[0]);
		  Wakeup_Idle();
		  LTC_STCOMM(2);
		  tempindex = 8;
		  indexpause = NUM_THERM_PER_MOD;
	  } else if (indexpause == NUM_THERM_PER_MOD) {
		  Wakeup_Idle();
		  LTC_WRCOMM(NUM_DEVICES, BMS_MUX_PAUSE[1]);
		  Wakeup_Idle();
		  LTC_STCOMM(2);
		  indexpause = 8;
		  tempindex = 0;
	  }

//		  uint16_t temp_Temperatures[NUM_THERM_TOTAL];
//		  memcpy(temp_Temperatures, modPackInfo.cell_temp, sizeof(temp_Temperatures));
//
//		  if (xQueueSend(TempQueueHandle, temp_Temperatures, 0) != pdPASS) {
//		      printf("Failed to send voltage array to temperature queue.\n");
//		  }

//	  // ミューテックス解放
//	  osMutexRelease(TempQueueMutexHandle);
//	  }
//	  else{
//		  printf("Failed to acquire temperature queue mutex!\n");
//	  }

  }
  /* USER CODE END StartReadTemp */
}

/* USER CODE BEGIN Header_StartCellSummaryVoltage */
/**
* @brief Function implementing the CellSummaryVolt thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCellSummaryVoltage */
void StartCellSummaryVoltage(void *argument)
{
  /* USER CODE BEGIN StartCellSummaryVoltage */
  /* Infinite loop */
  for(;;)
  {
	Cell_Summary_Voltage(&modPackInfo, &safetyFaults,
	  					&safetyWarnings, &safetyStates, &low_volt_hysteresis,
	  					&high_volt_hysteresis, &cell_imbalance_hysteresis);
    osDelay(205);
  }
  /* USER CODE END StartCellSummaryVoltage */
}

/* USER CODE BEGIN Header_StartCellSummaryTemperature */
/**
* @brief Function implementing the CellSummaryTemp thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCellSummaryTemperature */
void StartCellSummaryTemperature(void *argument)
{
  /* USER CODE BEGIN StartCellSummaryTemperature */
  /* Infinite loop */
  for(;;)
  {
	Cell_Summary_Temperature(&modPackInfo, &safetyFaults,&safetyWarnings);
    osDelay(2);
  }
  /* USER CODE END StartCellSummaryTemperature */
}

/* USER CODE BEGIN Header_StartStartBalance */
/**
* @brief Function implementing the StartBalance thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartStartBalance */
void StartStartBalance(void *argument)
{
  /* USER CODE BEGIN StartStartBalance */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartStartBalance */
}

/* USER CODE BEGIN Header_StartCANVolt */
/**
* @brief Function implementing the CANVolt thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANVolt */
void StartCANVolt(void *argument)
{
  /* USER CODE BEGIN StartCANVolt */
  /* Infinite loop */
  for(;;)
  {
	CAN_Send_Voltage(&msg, modPackInfo.cell_volt);
    osDelay(300);
  }
  /* USER CODE END StartCANVolt */
}

/* USER CODE BEGIN Header_StartCANTemp */
/**
* @brief Function implementing the CANTemp thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANTemp */
void StartCANTemp(void *argument)
{
  /* USER CODE BEGIN StartCANTemp */
  /* Infinite loop */
  for(;;)
  {
	CAN_Send_Temperature(&msg, modPackInfo.cell_temp);
    osDelay(300);
  }
  /* USER CODE END StartCANTemp */
}

/* USER CODE BEGIN Header_StartCANSummary */
/**
* @brief Function implementing the CANSummary thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANSummary */
void StartCANSummary(void *argument)
{
  /* USER CODE BEGIN StartCANSummary */
  /* Infinite loop */
  for(;;)
  {
	CAN_Send_Cell_Summary(&msg, &modPackInfo);
    osDelay(300);
  }
  /* USER CODE END StartCANSummary */
}

/* USER CODE BEGIN Header_StartCANFault */
/**
* @brief Function implementing the CANFault thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANFault */
void StartCANFault(void *argument)
{
  /* USER CODE BEGIN StartCANFault */
  /* Infinite loop */
  for(;;)
  {
	CAN_Send_Safety_Checker(&msg, &modPackInfo, &safetyFaults,
	  					&safetyWarnings, &safetyStates);
    osDelay(300);
  }
  /* USER CODE END StartCANFault */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

