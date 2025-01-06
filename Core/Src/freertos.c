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
/* Definitions for CAN_Signal_send */
osThreadId_t CAN_Signal_sendHandle;
const osThreadAttr_t CAN_Signal_send_attributes = {
  .name = "CAN_Signal_send",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Read_Volt */
osThreadId_t Read_VoltHandle;
const osThreadAttr_t Read_Volt_attributes = {
  .name = "Read_Volt",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Read_Temp */
osThreadId_t Read_TempHandle;
const osThreadAttr_t Read_Temp_attributes = {
  .name = "Read_Temp",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Cell_Summary */
osThreadId_t Cell_SummaryHandle;
const osThreadAttr_t Cell_Summary_attributes = {
  .name = "Cell_Summary",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Fault_Warning */
osThreadId_t Fault_WarningHandle;
const osThreadAttr_t Fault_Warning_attributes = {
  .name = "Fault_Warning",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Start_Balance */
osThreadId_t Start_BalanceHandle;
const osThreadAttr_t Start_Balance_attributes = {
  .name = "Start_Balance",
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
void Start_CAN_Signal_send(void *argument);
void Start_Read_Volt(void *argument);
void Start_Read_Temp(void *argument);
void Start_Cell_Summary(void *argument);
void Start_Fault_Warning_State(void *argument);
void Start_Start_Balance(void *argument);

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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of HartBeatLED */
  HartBeatLEDHandle = osThreadNew(StartHartBeatLED, NULL, &HartBeatLED_attributes);

  /* creation of CAN_Signal_send */
  CAN_Signal_sendHandle = osThreadNew(Start_CAN_Signal_send, NULL, &CAN_Signal_send_attributes);

  /* creation of Read_Volt */
  Read_VoltHandle = osThreadNew(Start_Read_Volt, NULL, &Read_Volt_attributes);

  /* creation of Read_Temp */
  Read_TempHandle = osThreadNew(Start_Read_Temp, NULL, &Read_Temp_attributes);

  /* creation of Cell_Summary */
  Cell_SummaryHandle = osThreadNew(Start_Cell_Summary, NULL, &Cell_Summary_attributes);

  /* creation of Fault_Warning */
  Fault_WarningHandle = osThreadNew(Start_Fault_Warning_State, NULL, &Fault_Warning_attributes);

  /* creation of Start_Balance */
  Start_BalanceHandle = osThreadNew(Start_Start_Balance, NULL, &Start_Balance_attributes);

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

/* USER CODE BEGIN Header_Start_CAN_Signal_send */
/**
* @brief Function implementing the CAN_Signal_send thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_CAN_Signal_send */
void Start_CAN_Signal_send(void *argument)
{
  /* USER CODE BEGIN Start_CAN_Signal_send */
  /* Infinite loop */
  for(;;)
  {
	CAN_Send_Safety_Checker(&msg, &modPackInfo, &safetyFaults, &safetyWarnings, &safetyStates);
	CAN_Send_Cell_Summary(&msg, &modPackInfo);
	CAN_Send_Voltage(&msg, modPackInfo.cell_volt);
	CAN_Send_Temperature(&msg, modPackInfo.cell_temp);
    osDelay(1);
  }
  /* USER CODE END Start_CAN_Signal_send */
}

/* USER CODE BEGIN Header_Start_Read_Volt */
/**
* @brief Function implementing the Read_Volt thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Read_Volt */
void Start_Read_Volt(void *argument)
{
  /* USER CODE BEGIN Start_Read_Volt */
  /* Infinite loop */
  for(;;)
  {
	  if (osMutexAcquire(VoltageQueueMutexHandle, osWaitForever) == osOK) {
		  Read_Volt(modPackInfo.cell_volt);

		  uint16_t temp_voltages[NUM_CELLS];
		  memcpy(temp_voltages, modPackInfo.cell_volt, sizeof(temp_voltages));

		  if (xQueueSend(VoltQueueHandle, temp_voltages, 0) != pdPASS) {
		      printf("Failed to send voltage array to voltage queue.\n");
		  }

	  // ミューテックス解放
	  osMutexRelease(VoltageQueueMutexHandle);
	  }
	  else{
		  printf("Failed to acquire mutex for voltage queue!\n");
	  }

	  osDelay(500);
  /* USER CODE END Start_Read_Volt */
}

/* USER CODE BEGIN Header_Start_Read_Temp */
/**
* @brief Function implementing the Read_Temp thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Read_Temp */
void Start_Read_Temp(void *argument)
{
  /* USER CODE BEGIN Start_Read_Temp */
  /* Infinite loop */
  for(;;)
  {
	  if (osMutexAcquire(TempQueueMutexHandle, osWaitForever) == osOK) {
		  for (uint8_t i = tempindex; i < indexpause; i++) {
			  Wakeup_Idle();
			  Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);
			  HAL_Delay(3);
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

		  uint16_t temp_Temperatures[NUM_THERM_TOTAL];
		  memcpy(temp_Temperatures, modPackInfo.cell_temp, sizeof(temp_Temperatures));

		  if (xQueueSend(TempQueueHandle, temp_Temperatures, 0) != pdPASS) {
		      printf("Failed to send voltage array to temperature queue.\n");
		  }

	  // ミューテックス解放
	  osMutexRelease(TempQueueMutexHandle);
	  }
	  else{
		  printf("Failed to acquire temperature queue mutex!\n");
	  }

	  osDelay(500);
  }
  /* USER CODE END Start_Read_Temp */
}

/* USER CODE BEGIN Header_Start_Cell_Summary */
/**
* @brief Function implementing the Cell_Summary thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Cell_Summary */
void Start_Cell_Summary(void *argument)
{
  /* USER CODE BEGIN Start_Cell_Summary */
  /* Infinite loop */
  for(;;)
  {
	  Cell_Summary(&modPackInfo);
	  osDelay(1);
  }
  /* USER CODE END Start_Cell_Summary */
}

/* USER CODE BEGIN Header_Start_Fault_Warning_State */
/**
* @brief Function implementing the Fault_Warning thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Fault_Warning_State */
void Start_Fault_Warning_State(void *argument)
{
  /* USER CODE BEGIN Start_Fault_Warning_State */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_Fault_Warning_State */
}

/* USER CODE BEGIN Header_Start_Start_Balance */
/**
* @brief Function implementing the Start_Balance thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Start_Balance */
void Start_Start_Balance(void *argument)
{
  /* USER CODE BEGIN Start_Start_Balance */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_Start_Balance */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

