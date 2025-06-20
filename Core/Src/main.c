/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "can.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "6811.h"
#include <stdio.h>
#include "module.h"
#include "safety.h"
#include "string.h"
#include <time.h>
#include "balance.h"
#include "soc.h"
#include <stdint.h>
#include "hv_sense.h"

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

/* USER CODE BEGIN PV */
typedef struct _GpioTimePacket {
    GPIO_TypeDef *gpio_port;  // Port
    uint16_t gpio_pin;        // Pin number
    uint32_t ts_prev;         // Previous timestamp
    uint32_t ts_curr;         // Current timestamp
} GpioTimePacket;
typedef struct _TimerPacket {
    uint32_t ts_prev;  // Previous timestamp
    uint32_t ts_curr;  // Current timestamp
    uint32_t delay;    // Amount to delay
} TimerPacket;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void GpioTimePacket_Init(GpioTimePacket *gtp, GPIO_TypeDef *port, uint16_t pin);
void TimerPacket_Init(TimerPacket *tp, uint32_t delay);
void GpioFixedToggle(GpioTimePacket *gtp, uint16_t update_ms);
// Returns 1 at every tp->delay interval
uint8_t TimerPacket_FixedPulse(TimerPacket *tp);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t BMS_MUX_PAUSE[2][6] = {{0x69, 0x28, 0x0F, 0x09, 0x7F, 0xF9},
                                      {0x69, 0x08, 0x0F, 0x09, 0x7F, 0xF9}};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    GpioTimePacket tp_led_heartbeat;
    TimerPacket cycleTimeCap;
    TimerPacket canReconnection;

    batteryModule modPackInfo;
	CANMessage msg;
	uint8_t safetyFaults = 0;
	uint8_t safetyWarnings = 0;
//	uint8_t moduleCounts = 0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM7_Init();
  MX_SPI1_Init();
  MX_CAN1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  CAN_SettingsInit(&msg);  // Start CAN at 0x00
    // Start timer
    GpioTimePacket_Init(&tp_led_heartbeat, MCU_HEARTBEAT_LED_GPIO_Port,
                        MCU_HEARTBEAT_LED_Pin);
    TimerPacket_Init(&cycleTimeCap, CYCLETIME_CAP);
    TimerPacket_Init(&canReconnection, CAN_RECONNECTION_CHECK);

    // Pull SPI1 nCS HIGH (deselect)
    LTC_nCS_High();

//	//Sending a fault signal and reseting it
//	HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
    ClearFaultSignal();	//those are for debug the charger and mobo

	//initializing variables
	uint8_t tempindex = 0;
	uint8_t indexpause = 8;

	Wakeup_Sleep();

    Read_Volt(modPackInfo.cell_volt);

    for (uint8_t i = 0; i < 8; i++) {
//				HAL_Delay(300);
        Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);

//				printf(" Cell: %d, Temp: %d\n", i, modPackInfo.cell_temp[i]);
    }
    LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX_PAUSE[0]);
    LTC_SPI_requestData(2);
//				HAL_Delay(1); //this delay is for stablize mux
    for (uint8_t i = 8; i < NUM_THERM_PER_MOD; i++) {
//				HAL_Delay(300);
        Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);

//				printf(" Cell: %d, Temp: %d\n", i, modPackInfo.cell_temp[i]);
    }
    LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX_PAUSE[1]);
    LTC_SPI_requestData(2);
//				HAL_Delay(1); //this delay is for stablize mux
    Balance_init(modPackInfo.balance_status);

    ReadHVInput(&modPackInfo);
    getSumPackVoltage(&modPackInfo);

	SOC_getInitialCharge(&modPackInfo);
	uint32_t prev_soc_time = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		GpioFixedToggle(&tp_led_heartbeat, LED_HEARTBEAT_DELAY_MS);
		if (TimerPacket_FixedPulse(&cycleTimeCap)) {
			 HAL_ADCEx_Calibration_Start(&hadc1);
			 HAL_ADCEx_Calibration_Start(&hadc2);
//		printf("hello\n");
			//reading cell voltages
//			printf("volt start\n");
			Read_Volt(modPackInfo.cell_volt);
//			printf("volt end\n");
//			printf("Cell voltages:\n");
//			for (int i = 0; i < NUM_CELLS; i++) {
//			    printf("Cell %d: %u mV\n", i + 1, modPackInfo.cell_volt[i]);
//			}

			//reading cell temperatures
			for (uint8_t i = tempindex; i < indexpause; i++) {
//				HAL_Delay(300);
				Read_Temp(i, modPackInfo.cell_temp, modPackInfo.read_auxreg);

//				printf(" Cell: %d, Temp: %d\n", i, modPackInfo.cell_temp[i]);
			}
			if (indexpause == 8) {
				LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX_PAUSE[0]);
				LTC_SPI_requestData(2);
				tempindex = 8;
				indexpause = NUM_THERM_PER_MOD;
//				HAL_Delay(1); //this delay is for stablize mux
			}
			else if (indexpause == NUM_THERM_PER_MOD) {
				Read_Pressure(&modPackInfo);
				Read_Humidity(&modPackInfo);
				Read_Atmos_Temp(&modPackInfo);
				Get_Dew_Point(&modPackInfo);
				LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX_PAUSE[1]);
				LTC_SPI_requestData(2);
				indexpause = 8;
				tempindex = 0;
//				HAL_Delay(1); //this delay is for stablize mux
			}

//			for(int i = 0; i < NUM_THERM_TOTAL; i++){
//				printf("Temp[%d]: %d\n",i, modPackInfo.cell_temp[i]);
//			}
//			printf("pack volt start\n");
			ReadHVInput(&modPackInfo);
			getSumPackVoltage(&modPackInfo);
//			printf("pack volt end\n");

			SOC_updateCharge(&modPackInfo,(HAL_GetTick() - prev_soc_time));
			prev_soc_time = HAL_GetTick();
			//getting the summary of all cells in the pack
            Cell_Voltage_Fault(	&modPackInfo, &safetyFaults, &safetyWarnings);
			Cell_Temperature_Fault(&modPackInfo, &safetyFaults, &safetyWarnings);
//			Passive balancing is called unless a fault has occurred
//			if (safetyFaults == 0 && BALANCE
//					&& ((modPackInfo.cell_volt_highest
//							- modPackInfo.cell_volt_lowest) > 50)) {
//				Start_Balance((uint16_t*) modPackInfo.cell_volt,
//				NUM_DEVICES, modPackInfo.cell_volt_lowest);

//			} else if (BALANCE) {
//				End_Balance(&safetyFaults);
//			}
            if(modPackInfo.cell_difference > BALANCE_THRESHOLD){
				Start_Balance(modPackInfo.cell_volt, modPackInfo.cell_volt_lowest, modPackInfo.balance_status);
			}

			End_Balance(modPackInfo.balance_status);//end the balance if CAN RX recieve 0


			//calling all CAN realated methods
//			printf("CAN start\n");
			if(TimerPacket_FixedPulse(&canReconnection)){
				can_skip_flag = 0;
			}
			CAN_Send_Safety_Checker(&msg, &modPackInfo, &safetyFaults, &safetyWarnings);
			CAN_Send_Cell_Summary(&msg, &modPackInfo);
			CAN_Send_Voltage(&msg, modPackInfo.cell_volt);
			CAN_Send_Temperature(&msg, modPackInfo.cell_temp, modPackInfo.pressure, modPackInfo.atmos_temp, modPackInfo.humidity, modPackInfo.dew_point);
			CAN_Send_SOC(&msg, &modPackInfo, MAX_BATTERY_CAPACITY);
			CAN_Send_Balance_Status(&msg, modPackInfo.balance_status);
		}
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_ON;
  RCC_OscInitStruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
  RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/* USER CODE BEGIN 4 */
// Initialize struct values
// Will initialize GPIO to LOW!
void GpioTimePacket_Init(GpioTimePacket *gtp, GPIO_TypeDef *port,
                         uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);  // Set GPIO LOW
    gtp->gpio_port = port;
    gtp->gpio_pin = pin;
    gtp->ts_prev = 0;  // Init to 0
    gtp->ts_curr = 0;  // Init to 0
}
// update_ms = update after X ms
void GpioFixedToggle(GpioTimePacket *gtp, uint16_t update_ms) {
    gtp->ts_curr = HAL_GetTick();  // Record current timestamp
    if (gtp->ts_curr - gtp->ts_prev > update_ms) {
        HAL_GPIO_TogglePin(gtp->gpio_port, gtp->gpio_pin);  // Toggle GPIO
        gtp->ts_prev = gtp->ts_curr;
    }
}
// Initialize struct values
// Will initialize GPIO to LOW!
void TimerPacket_Init(TimerPacket *tp, uint32_t delay) {
    tp->ts_prev = 0;    // Init to 0
    tp->ts_curr = 0;    // Init to 0
    tp->delay = delay;  // Init to user value
}
// update_ms = update after X ms
uint8_t TimerPacket_FixedPulse(TimerPacket *tp) {
    tp->ts_curr = HAL_GetTick();  // Record current timestamp
    if (tp->ts_curr - tp->ts_prev > tp->delay) {
        tp->ts_prev = tp->ts_curr;  // Update prev timestamp to current
        return 1;                   // Enact event (time interval is a go)
    }
    return 0;  // Do not enact event
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state
     */
    __disable_irq();
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
