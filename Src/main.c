/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/main.c
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "timers.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 1000 )
#define CURRENT_TIME_DATE
#define RTC_ALARM
//#define COUNT_SEC
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

RTC_HandleTypeDef hrtc;

/* Definitions for THREAD1 */
osThreadId_t THREAD1Handle;
const osThreadAttr_t THREAD1_attributes = {
  .name = "THREAD1",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for THREAD2 */
osThreadId_t THREAD2Handle;
const osThreadAttr_t THREAD2_attributes = {
  .name = "THREAD2",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* USER CODE BEGIN PV */
__IO uint32_t OsStatus = 0;
I2C_HandleTypeDef hi2c1;
TimerHandle_t xAutoReloadTimer;
char time[10];
char date[10];
char alarm[10];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_ICACHE_Init(void);
static void MX_RTC_Init(void);
void LED_Thread1(void *argument);
void LED_Thread2(void *argument);

/* USER CODE BEGIN PFP */
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
//static void prvTimerCallback( TimerHandle_t xTimer );
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* Initialize LEDs */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_ICACHE_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  MX_GPIO_Init();
   MX_I2C1_Init();
   ssd1306_Init();

  /* USER CODE END 2 */

  /* Init scheduler */
  //osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /*xAutoReloadTimer = xTimerCreate("AutoReload",AUTO_RELOAD_TIMER_PERIOD,pdTRUE,0,prvTimerCallback);

     if(  xAutoReloadTimer != NULL  )
      {
        xTimerStart( xAutoReloadTimer, 0 );
      }*/
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of THREAD1 */
  //THREAD1Handle = osThreadNew(LED_Thread1, NULL, &THREAD1_attributes);

  /* creation of THREAD2 */
  //THREAD2Handle = osThreadNew(LED_Thread2, NULL, &THREAD2_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  //osKernelStart();
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  	  ssd1306_rtc();
	  	  HAL_Delay(500);

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 55;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_PrivilegeStateTypeDef privilegeState = {0};
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x23;
  sTime.Minutes = 0x59;
  sTime.Seconds = 0x50;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JULY;
  sDate.Date = 0x24;
  sDate.Year = 0x23;
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x25;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/* USER CODE BEGIN 4 */

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
    	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	    {
	      Error_Handler();
	    }

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x40505681;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief GPIO Initialization Function for I2C1
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_I2C1_CLK_ENABLE();
      GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
 * @brief Timer callback function for handling SSD1306 display operations.
 *
 * This function is a callback for a FreeRTOS timer. It updates the SSD1306 display
 * with the current count and calls the 'ssd1306_Count()' function.
 *
 * @param xTimer The handle to the timer that triggered the callback.
 * @retval None
 */
#ifdef COUNT_SEC
static void prvTimerCallback( TimerHandle_t xTimer )
{
    static int count=0;
    ssd1306_Name();
    count++;
    ssd1306_Count(count);
 }
#endif


/**
 * @brief Update and display the current time and date on the SSD1306 OLED display.
 *
 * This function retrieves the current time and date from the RTC peripheral and
 * formats them as strings to be displayed on the SSD1306 OLED display. The time
 * is displayed in the format "HH:MM:SS" (hours, minutes, seconds), and the date
 * is displayed in the format "DD-MM-YY" (day, month, year).
 * @param None
 * @retval None
 */

#ifdef 	CURRENT_TIME_DATE

void ssd1306_rtc(void)
{
	RTC_TimeTypeDef Time={0};
	RTC_DateTypeDef Date={0};
	HAL_RTC_GetTime(&hrtc,&Time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&Date,RTC_FORMAT_BIN);
	snprintf((char*)time, sizeof(time), "%02d:%02d:%02d", Time.Hours, Time.Minutes,Time.Seconds);
	snprintf((char*)date, sizeof(date), "%02d-%02d-%02d", Date.Date, Date.Month,Date.Year);

	ssd1306_Fill(White);
	ssd1306_SetCursor(2, 0);
	ssd1306_WriteString(time, Font_11x18, Black);
	ssd1306_SetCursor(2, 18);
    ssd1306_WriteString(date, Font_11x18, Black);
	ssd1306_UpdateScreen();
}
#endif

/**
 * @brief RTC Alarm A Event Callback function.
 *
 * This function is called when the RTC Alarm A event occurs. It retrieves the
 * alarm configuration and formats the alarm time as a string. The formatted alarm
 * time is then displayed on an SSD1306 OLED display.
 *
 * @param[in] hrtc A pointer to the RTC handle, which contains the configuration
 *                and state information of the RTC peripheral.
 */
#ifdef RTC_ALARM

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	  RTC_AlarmTypeDef Alarm={0};
	  HAL_RTC_GetAlarm(hrtc,&Alarm,RTC_ALARM_A,RTC_FORMAT_BIN);
	  snprintf((char*)alarm, sizeof(alarm), "%02d:%02d:%02d", Alarm.AlarmTime.Hours, Alarm.AlarmTime.Minutes,Alarm.AlarmTime.Seconds);

	  //ssd1306_Fill(White);
	  ssd1306_SetCursor(2, 36);
	  ssd1306_WriteString("Alarm", Font_11x18, Black);
	  ssd1306_SetCursor(77, 36);
	  ssd1306_WriteString(alarm, Font_11x18, Black);
}
#endif


/* USER CODE END 4 */

/* USER CODE BEGIN Header_LED_Thread1 */
/**
  * @brief  Function implementing the THREAD1 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_LED_Thread1 */
void LED_Thread1(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_LED_Thread2 */
/**
* @brief Function implementing the THREAD2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LED_Thread2 */
void LED_Thread2(void *argument)
{
  /* USER CODE BEGIN LED_Thread2 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END LED_Thread2 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
