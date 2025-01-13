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
#include "main.h"
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
/* Definitions for CANVoltSummary */
osThreadId_t CANVoltSummaryHandle;
const osThreadAttr_t CANVoltSummary_attributes = {
  .name = "CANVoltSummary",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANTempSummary */
osThreadId_t CANTempSummaryHandle;
const osThreadAttr_t CANTempSummary_attributes = {
  .name = "CANTempSummary",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TempQueue */
osMessageQueueId_t TempQueueHandle;
const osMessageQueueAttr_t TempQueue_attributes = {
  .name = "TempQueue"
};
/* Definitions for VoltQueue */
osMessageQueueId_t VoltQueueHandle;
const osMessageQueueAttr_t VoltQueue_attributes = {
  .name = "VoltQueue"
};
/* Definitions for TempSummaryQueue */
osMessageQueueId_t TempSummaryQueueHandle;
const osMessageQueueAttr_t TempSummaryQueue_attributes = {
  .name = "TempSummaryQueue"
};
/* Definitions for VoltSummaryQueue */
osMessageQueueId_t VoltSummaryQueueHandle;
const osMessageQueueAttr_t VoltSummaryQueue_attributes = {
  .name = "VoltSummaryQueue"
};
/* Definitions for VoltageQueueMutex */
osMutexId_t VoltageQueueMutexHandle;
const osMutexAttr_t VoltageQueueMutex_attributes = {
  .name = "VoltageQueueMutex"
};
/* Definitions for TempQueueMutex */
osMutexId_t TempQueueMutexHandle;
const osMutexAttr_t TempQueueMutex_attributes = {
  .name = "TempQueueMutex"
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
void StartCANVoltSummary(void *argument);
void StartCANTempSummary(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of VoltageQueueMutex */
  VoltageQueueMutexHandle = osMutexNew(&VoltageQueueMutex_attributes);

  /* creation of TempQueueMutex */
  TempQueueMutexHandle = osMutexNew(&TempQueueMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of TempQueue */
  TempQueueHandle = osMessageQueueNew (5, 24, &TempQueue_attributes);

  /* creation of VoltQueue */
  VoltQueueHandle = osMessageQueueNew (5, 24, &VoltQueue_attributes);

  /* creation of TempSummaryQueue */
  TempSummaryQueueHandle = osMessageQueueNew (5, 24, &TempSummaryQueue_attributes);

  /* creation of VoltSummaryQueue */
  VoltSummaryQueueHandle = osMessageQueueNew (5, 24, &VoltSummaryQueue_attributes);

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

  /* creation of CANVoltSummary */
  CANVoltSummaryHandle = osThreadNew(StartCANVoltSummary, NULL, &CANVoltSummary_attributes);

  /* creation of CANTempSummary */
  CANTempSummaryHandle = osThreadNew(StartCANTempSummary, NULL, &CANTempSummary_attributes);

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
	  GpioFixedToggle(&tp_led_heartbeat, 1000); // 1秒間隔�?�トグル
	  osDelay(1000); // 短�?��?�延�?�タスクスケジューリングを譲る

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
    osDelay(1);
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
    osDelay(1);
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
    osDelay(1);
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
    osDelay(1);
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
    osDelay(1);
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
    osDelay(1);
  }
  /* USER CODE END StartCANTemp */
}

/* USER CODE BEGIN Header_StartCANVoltSummary */
/**
* @brief Function implementing the CANVoltSummary thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANVoltSummary */
void StartCANVoltSummary(void *argument)
{
  /* USER CODE BEGIN StartCANVoltSummary */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCANVoltSummary */
}

/* USER CODE BEGIN Header_StartCANTempSummary */
/**
* @brief Function implementing the CANTempSummary thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANTempSummary */
void StartCANTempSummary(void *argument)
{
  /* USER CODE BEGIN StartCANTempSummary */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCANTempSummary */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

