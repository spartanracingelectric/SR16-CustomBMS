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
#include "string.h"
#include "6811.h"
#include "print.h"
#include "module.h"
#include "safety.h"
#include "can.h"
#include "balance.h"
#include "stdio.h"
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
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
	GpioTimePacket tp_led_heartbeat;
	TimerPacket timerpacket_ltc;

	struct batteryModule modPackInfo;
	struct CANMessage msg;
	uint8_t safetyFaults = 0;
	uint8_t safetyWarnings = 0;
	uint8_t safetyStates = 0;
	uint8_t tempindex = 0;
	uint8_t indexpause = 8;
	uint8_t low_volt_hysteresis = 0;
	uint8_t high_volt_hysteresis = 0;
	uint8_t cell_imbalance_hysteresis = 0;

	static uint8_t BMS_MUX_PAUSE[2][6] = { { 0x69, 0x28, 0x0F, 0x09, 0x7F, 0xF9 }, {
			0x69, 0x08, 0x0F, 0x09, 0x7F, 0xF9 } };
/* USER CODE END Variables */
/* Definitions for HartBeatLED */
osThreadId_t HartBeatLEDHandle;
const osThreadAttr_t HartBeatLED_attributes = {
  .name = "HartBeatLED",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ReadVolt */
osThreadId_t ReadVoltHandle;
const osThreadAttr_t ReadVolt_attributes = {
  .name = "ReadVolt",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for ReadTemp */
osThreadId_t ReadTempHandle;
const osThreadAttr_t ReadTemp_attributes = {
  .name = "ReadTemp",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for CellSummaryVolt */
osThreadId_t CellSummaryVoltHandle;
const osThreadAttr_t CellSummaryVolt_attributes = {
  .name = "CellSummaryVolt",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CellSummaryTemp */
osThreadId_t CellSummaryTempHandle;
const osThreadAttr_t CellSummaryTemp_attributes = {
  .name = "CellSummaryTemp",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for StartBalance */
osThreadId_t StartBalanceHandle;
const osThreadAttr_t StartBalance_attributes = {
  .name = "StartBalance",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANVolt */
osThreadId_t CANVoltHandle;
const osThreadAttr_t CANVolt_attributes = {
  .name = "CANVolt",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CANTemp */
osThreadId_t CANTempHandle;
const osThreadAttr_t CANTemp_attributes = {
  .name = "CANTemp",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CANSummary */
osThreadId_t CANSummaryHandle;
const osThreadAttr_t CANSummary_attributes = {
  .name = "CANSummary",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANFault */
osThreadId_t CANFaultHandle;
const osThreadAttr_t CANFault_attributes = {
  .name = "CANFault",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void GpioTimePacket_Init(GpioTimePacket *gtp, GPIO_TypeDef *port, uint16_t pin);
void TimerPacket_Init(TimerPacket *tp, uint32_t delay);
void GpioFixedToggle(GpioTimePacket *gtp, uint16_t update_ms);
uint8_t TimerPacket_FixedPulse(TimerPacket *tp);
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
	  MX_GPIO_Init();
	  MX_ADC1_Init();
	  MX_ADC2_Init();
	  MX_TIM7_Init();
	  MX_SPI1_Init();
	  MX_CAN1_Init();
	  MX_USART1_UART_Init();

	  CAN_SettingsInit(&msg); // Start CAN at 0x00
	  	//Start timer
	  	GpioTimePacket_Init(&tp_led_heartbeat, MCU_HEARTBEAT_LED_GPIO_Port,
	  	MCU_HEARTBEAT_LED_Pin);
	  	TimerPacket_Init(&timerpacket_ltc, LTC_DELAY);
	  	//Pull SPI1 nCS HIGH (deselect)
	  	LTC_nCS_High();

	  	//Sending a fault signal and reseting it
	  	HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
	  	osDelay(500);
	  	HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_RESET);

	  	//initializing variables


	  	//reading cell voltages
	  	Wakeup_Sleep();
	  	Read_Volt(modPackInfo.cell_volt);

	  	//reading cell temperatures
	  	Wakeup_Sleep();
	  	for (uint8_t i = tempindex; i < indexpause; i++) {
	  		Wakeup_Idle();
	  		Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);
	  		osDelay(3);
	  	}
	  	Wakeup_Idle();
	  	LTC_WRCOMM(NUM_DEVICES, BMS_MUX_PAUSE[0]);
	  	Wakeup_Idle();
	  	LTC_STCOMM(2);

	  	Wakeup_Sleep();
	  	for (uint8_t i = indexpause; i < NUM_THERM_PER_MOD; i++) {
	  		Wakeup_Idle();
	  		Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);
	  		osDelay(3);
	  	}
	  	Wakeup_Idle();
	  	LTC_WRCOMM(NUM_DEVICES, BMS_MUX_PAUSE[1]);
	  	Wakeup_Idle();
	  	LTC_STCOMM(2);


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
	  GpioFixedToggle(&tp_led_heartbeat, LED_HEARTBEAT_DELAY_MS); // Toggle heat beat LED every 1 sec
	  printf("hello");
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
	  Wakeup_Sleep();
	  Read_Volt(modPackInfo.cell_volt);
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
	  Wakeup_Sleep();

	  for (uint8_t i = tempindex; i < indexpause; i++) {
		  Wakeup_Idle();
		  Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);
	  }
	  osDelay(2);
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
    osDelay(300);
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
    osDelay(300);
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
    osDelay(400);
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
    osDelay(400);
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
    osDelay(400);
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
    osDelay(400);
  }
  /* USER CODE END StartCANFault */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
//Initialize struct values
//Will initialize GPIO to LOW!
void GpioTimePacket_Init(GpioTimePacket *gtp, GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); //Set GPIO LOW
	gtp->gpio_port = port;
	gtp->gpio_pin = pin;
	gtp->ts_prev = 0; //Init to 0
	gtp->ts_curr = 0; //Init to 0
}
//update_ms = update after X ms
void GpioFixedToggle(GpioTimePacket *gtp, uint16_t update_ms) {
	gtp->ts_curr = HAL_GetTick(); //Record current timestamp
	if (gtp->ts_curr - gtp->ts_prev > update_ms) {
		HAL_GPIO_TogglePin(gtp->gpio_port, gtp->gpio_pin); // Toggle GPIO
		gtp->ts_prev = gtp->ts_curr;
	}
}
//Initialize struct values
//Will initialize GPIO to LOW!
void TimerPacket_Init(TimerPacket *tp, uint32_t delay) {
	tp->ts_prev = 0;		//Init to 0
	tp->ts_curr = 0; 		//Init to 0
	tp->delay = delay;	//Init to user value
}
//update_ms = update after X ms
uint8_t TimerPacket_FixedPulse(TimerPacket *tp) {
	tp->ts_curr = HAL_GetTick(); //Record current timestamp
	if (tp->ts_curr - tp->ts_prev > tp->delay) {
		tp->ts_prev = tp->ts_curr; //Update prev timestamp to current
		return 1; //Enact event (time interval is a go)
	}
	return 0; //Do not enact event
}
/* USER CODE END Application */

