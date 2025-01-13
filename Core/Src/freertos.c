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
#include "safety.h"
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

	  	  osDelay(500);
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

  osDelay(500);
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
	Cell_Summary_Temperature(&modPackInfo, &safetyFaults,&safetyWarnings);
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
	CAN_Send_Voltage(&msg, modPackInfo.cell_volt);
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
	CAN_Send_Temperature(&msg, modPackInfo.cell_temp);
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
	CAN_Send_Cell_Summary(&msg, &modPackInfo);
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





///* USER CODE BEGIN Header */
///**
//  ******************************************************************************
//  * File Name          : freertos.c
//  * Description        : Code for freertos applications
//  ******************************************************************************
//  * @attention
//  *
//  * Copyright (c) 2024 STMicroelectronics.
//  * All rights reserved.
//  *
//  * This software is licensed under terms that can be found in the LICENSE file
//  * in the root directory of this software component.
//  * If no LICENSE file comes with this software, it is provided AS-IS.
//  *
//  ******************************************************************************
//  */
///* USER CODE END Header */
//
///* Includes ------------------------------------------------------------------*/
//#include "FreeRTOS.h"
//#include "task.h"
//#include "main.h"
//#include "cmsis_os.h"
//
///* Private includes ----------------------------------------------------------*/
///* USER CODE BEGIN Includes */
//typedef struct {
//    uint8_t tempIndex;            // サーミスタインデックス
//    uint16_t readTemp[NUM_DEVICES]; // デ�?イス�?��?��?�温度データ
//    uint16_t readAuxReg[NUM_DEVICES * NUM_AUX_GROUP]; // 補助レジスタデータ
//} TempData_t;
//struct batteryModule modPackInfo;
//struct CANMessage msg;
//uint8_t safetyFaults = 0;
//uint8_t safetyWarnings = 0;
//uint8_t safetyStates = 0;
//
///* USER CODE END Includes */
//
///* Private typedef -----------------------------------------------------------*/
///* USER CODE BEGIN PTD */
//
///* USER CODE END PTD */
//
///* Private define ------------------------------------------------------------*/
///* USER CODE BEGIN PD */
//
///* USER CODE END PD */
//
///* Private macro -------------------------------------------------------------*/
///* USER CODE BEGIN PM */
//
///* USER CODE END PM */
//
///* Private variables ---------------------------------------------------------*/
///* USER CODE BEGIN Variables */
//
///* USER CODE END Variables */
///* Definitions for HartBeatLED */
//osThreadId_t HartBeatLEDHandle;
//const osThreadAttr_t HartBeatLED_attributes = {
//  .name = "HartBeatLED",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityNormal,
//};
///* Definitions for CAN_Signal_send */
//osThreadId_t CAN_Signal_sendHandle;
//const osThreadAttr_t CAN_Signal_send_attributes = {
//  .name = "CAN_Signal_send",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityLow,
//};
///* Definitions for Read_Volt */
//osThreadId_t Read_VoltHandle;
//const osThreadAttr_t Read_Volt_attributes = {
//  .name = "Read_Volt",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityNormal,
//};
///* Definitions for Read_Temp */
//osThreadId_t Read_TempHandle;
//const osThreadAttr_t Read_Temp_attributes = {
//  .name = "Read_Temp",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityNormal,
//};
///* Definitions for Cell_Summary */
//osThreadId_t Cell_SummaryHandle;
//const osThreadAttr_t Cell_Summary_attributes = {
//  .name = "Cell_Summary",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityHigh,
//};
///* Definitions for Fault_Warning */
//osThreadId_t Fault_WarningHandle;
//const osThreadAttr_t Fault_Warning_attributes = {
//  .name = "Fault_Warning",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityHigh,
//};
///* Definitions for Start_Balance */
//osThreadId_t Start_BalanceHandle;
//const osThreadAttr_t Start_Balance_attributes = {
//  .name = "Start_Balance",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityLow,
//};
///* Definitions for TempQueue */
//osMessageQueueId_t TempQueueHandle;
//const osMessageQueueAttr_t TempQueue_attributes = {
//  .name = "TempQueue"
//};
///* Definitions for VoltQueue */
//osMessageQueueId_t VoltQueueHandle;
//const osMessageQueueAttr_t VoltQueue_attributes = {
//  .name = "VoltQueue"
//};
///* Definitions for VoltageQueueMutex */
//osMutexId_t VoltageQueueMutexHandle;
//const osMutexAttr_t VoltageQueueMutex_attributes = {
//  .name = "VoltageQueueMutex"
//};
///* Definitions for TempQueueMutex */
//osMutexId_t TempQueueMutexHandle;
//const osMutexAttr_t TempQueueMutex_attributes = {
//  .name = "TempQueueMutex"
//};
//
///* Private function prototypes -----------------------------------------------*/
///* USER CODE BEGIN FunctionPrototypes */
//
///* USER CODE END FunctionPrototypes */
//
//void StartHartBeatLED(void *argument);
//void Start_CAN_Signal_send(void *argument);
//void Start_Read_Volt(void *argument);
//void Start_Read_Temp(void *argument);
//void Start_Cell_Summary(void *argument);
//void Start_Fault_Warning_State(void *argument);
//void Start_Start_Balance(void *argument);
//
//void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */
//
///**
//  * @brief  FreeRTOS initialization
//  * @param  None
//  * @retval None
//  */
//void MX_FREERTOS_Init(void) {
//  /* USER CODE BEGIN Init */
//
//  /* USER CODE END Init */
//  /* Create the mutex(es) */
//  /* creation of VoltageQueueMutex */
//  VoltageQueueMutexHandle = osMutexNew(&VoltageQueueMutex_attributes);
//
//  /* creation of TempQueueMutex */
//  TempQueueMutexHandle = osMutexNew(&TempQueueMutex_attributes);
//
//  /* USER CODE BEGIN RTOS_MUTEX */
//  /* add mutexes, ... */
//  /* USER CODE END RTOS_MUTEX */
//
//  /* USER CODE BEGIN RTOS_SEMAPHORES */
//  /* add semaphores, ... */
//  /* USER CODE END RTOS_SEMAPHORES */
//
//  /* USER CODE BEGIN RTOS_TIMERS */
//  /* start timers, add new ones, ... */
//  /* USER CODE END RTOS_TIMERS */
//
//  /* Create the queue(s) */
//  /* creation of TempQueue */
//  TempQueueHandle = osMessageQueueNew (5, 24, &TempQueue_attributes);
//
//  /* creation of VoltQueue */
//  VoltQueueHandle = osMessageQueueNew (5, 24, &VoltQueue_attributes);
//
//  /* USER CODE BEGIN RTOS_QUEUES */
//  /* add queues, ... */
//  /* USER CODE END RTOS_QUEUES */
//
//  /* Create the thread(s) */
//  /* creation of HartBeatLED */
//  HartBeatLEDHandle = osThreadNew(StartHartBeatLED, NULL, &HartBeatLED_attributes);
//
//  /* creation of CAN_Signal_send */
//  CAN_Signal_sendHandle = osThreadNew(Start_CAN_Signal_send, NULL, &CAN_Signal_send_attributes);
//
//  /* creation of Read_Volt */
//  Read_VoltHandle = osThreadNew(Start_Read_Volt, NULL, &Read_Volt_attributes);
//
//  /* creation of Read_Temp */
//  Read_TempHandle = osThreadNew(Start_Read_Temp, NULL, &Read_Temp_attributes);
//
//  /* creation of Cell_Summary */
//  Cell_SummaryHandle = osThreadNew(Start_Cell_Summary, NULL, &Cell_Summary_attributes);
//
//  /* creation of Fault_Warning */
//  Fault_WarningHandle = osThreadNew(Start_Fault_Warning_State, NULL, &Fault_Warning_attributes);
//
//  /* creation of Start_Balance */
//  Start_BalanceHandle = osThreadNew(Start_Start_Balance, NULL, &Start_Balance_attributes);
//
//  /* USER CODE BEGIN RTOS_THREADS */
//  /* add threads, ... */
//  /* USER CODE END RTOS_THREADS */
//
//  /* USER CODE BEGIN RTOS_EVENTS */
//  /* add events, ... */
//  /* USER CODE END RTOS_EVENTS */
//
//}
//
///* USER CODE BEGIN Header_StartHartBeatLED */
///**
//  * @brief  Function implementing the HartBeatLED thread.
//  * @param  argument: Not used
//  * @retval None
//  */
///* USER CODE END Header_StartHartBeatLED */
//void StartHartBeatLED(void *argument)
//{
//  /* USER CODE BEGIN StartHartBeatLED */
//  /* Infinite loop */
//  for(;;)
//  {
//	  GpioFixedToggle(&tp_led_heartbeat, 1000); // 1秒間隔�?�トグル
//	  osDelay(1000); // 短�?��?�延�?�タスクスケジューリングを譲る
//
//  }
//  /* USER CODE END StartHartBeatLED */
//}
//
///* USER CODE BEGIN Header_Start_CAN_Signal_send */
///**
//* @brief Function implementing the CAN_Signal_send thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Start_CAN_Signal_send */
//void Start_CAN_Signal_send(void *argument)
//{
//  /* USER CODE BEGIN Start_CAN_Signal_send */
//  /* Infinite loop */
//  for(;;)
//  {
//	CAN_Send_Safety_Checker(&msg, &modPackInfo, &safetyFaults, &safetyWarnings, &safetyStates);
//	CAN_Send_Cell_Summary(&msg, &modPackInfo);
//	CAN_Send_Voltage(&msg, modPackInfo.cell_volt);
//	CAN_Send_Temperature(&msg, modPackInfo.cell_temp);
//    osDelay(1);
//  }
//  /* USER CODE END Start_CAN_Signal_send */
//}
//
///* USER CODE BEGIN Header_Start_Read_Volt */
///**
//* @brief Function implementing the Read_Volt thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Start_Read_Volt */
//void Start_Read_Volt(void *argument)
//{
//  /* USER CODE BEGIN Start_Read_Volt */
//  /* Infinite loop */
//  for(;;)
//  {
////	  if (osMutexAcquire(VoltageQueueMutexHandle, osWaitForever) == osOK) {
////		  Read_Volt(modPackInfo.cell_volt);
//
//		  uint16_t temp_voltages[NUM_CELLS];
////		  memcpy(temp_voltages, modPackInfo.cell_volt, sizeof(temp_voltages));
//
////		  if (xQueueSend(VoltQueueHandle, temp_voltages, 0) != pdPASS) {
////		      printf("Failed to send voltage array to voltage queue.\n");
////		  }
//
//	  // ミューテックス解放
////	  osMutexRelease(VoltageQueueMutexHandle);
////	  }
////	  else{
////		  printf("Failed to acquire mutex for voltage queue!\n");
////	  }
//
//	  osDelay(500);
//  /* USER CODE END Start_Read_Volt */
//}
//
///* USER CODE BEGIN Header_Start_Read_Temp */
///**
//* @brief Function implementing the Read_Temp thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Start_Read_Temp */
//void Start_Read_Temp(void *argument)
//{
//  /* USER CODE BEGIN Start_Read_Temp */
//  /* Infinite loop */
//  for(;;)
//  {
////	  if (osMutexAcquire(TempQueueMutexHandle, osWaitForever) == osOK) {
//		  for (uint8_t i = tempindex; i < indexpause; i++) {
//			  Wakeup_Idle();
//			  Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);
//			  osDelay(2);
//		  }
//		  if (indexpause == 8) {
//			  Wakeup_Idle();
//			  LTC_WRCOMM(NUM_DEVICES, BMS_MUX_PAUSE[0]);
//			  Wakeup_Idle();
//			  LTC_STCOMM(2);
//			  tempindex = 8;
//			  indexpause = NUM_THERM_PER_MOD;
//		  } else if (indexpause == NUM_THERM_PER_MOD) {
//			  Wakeup_Idle();
//			  LTC_WRCOMM(NUM_DEVICES, BMS_MUX_PAUSE[1]);
//			  Wakeup_Idle();
//			  LTC_STCOMM(2);
//			  indexpause = 8;
//			  tempindex = 0;
//		  }
//
////		  uint16_t temp_Temperatures[NUM_THERM_TOTAL];
////		  memcpy(temp_Temperatures, modPackInfo.cell_temp, sizeof(temp_Temperatures));
////
////		  if (xQueueSend(TempQueueHandle, temp_Temperatures, 0) != pdPASS) {
////		      printf("Failed to send voltage array to temperature queue.\n");
////		  }
//
////	  // ミューテックス解放
////	  osMutexRelease(TempQueueMutexHandle);
////	  }
////	  else{
////		  printf("Failed to acquire temperature queue mutex!\n");
////	  }
//
//	  osDelay(500);
//  }
//  /* USER CODE END Start_Read_Temp */
//}
//
///* USER CODE BEGIN Header_Start_Cell_Summary */
///**
//* @brief Function implementing the Cell_Summary thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Start_Cell_Summary */
//void Start_Cell_Summary(void *argument)
//{
//  /* USER CODE BEGIN Start_Cell_Summary */
//  /* Infinite loop */
//  for(;;)
//  {
//	  Cell_Summary(&modPackInfo);
//	  osDelay(1);
//  }
//  /* USER CODE END Start_Cell_Summary */
//}
//
///* USER CODE BEGIN Header_Start_Fault_Warning_State */
///**
//* @brief Function implementing the Fault_Warning thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Start_Fault_Warning_State */
//void Start_Fault_Warning_State(void *argument)
//{
//  /* USER CODE BEGIN Start_Fault_Warning_State */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END Start_Fault_Warning_State */
//}
//
///* USER CODE BEGIN Header_Start_Start_Balance */
///**
//* @brief Function implementing the Start_Balance thread.
//* @param argument: Not used
//* @retval None
//*/
///* USER CODE END Header_Start_Start_Balance */
//void Start_Start_Balance(void *argument)
//{
//  /* USER CODE BEGIN Start_Start_Balance */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
//  /* USER CODE END Start_Start_Balance */
//}
//
///* Private application code --------------------------------------------------*/
///* USER CODE BEGIN Application */
//
///* USER CODE END Application */
//
//
