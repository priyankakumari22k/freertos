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
#include "stdio.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_DOUBLEWORD_TYPEPROGRAM


I2C_HandleTypeDef hi2c1;
#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_126   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_126 + FLASH_PAGE_SIZE - 1)   /* End @ of user Flash area */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/


/* USER CODE BEGIN PV */
__IO uint32_t OsStatus = 0;

//FLASH_OBProgramInitTypeDef OptionsBytesStruct, OptionsBytesStruct2;

//uint32_t Address=0x08000000;
uint8_t i;
uint64_t data = (uint64_t)0x1111111111111111;
uint64_t data1=(uint64_t)0x1234567812345678 ;
uint64_t rdata;
uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
uint32_t Address = ADDR_FLASH_PAGE_126;
uint32_t PageError = 0;

__IO uint32_t MemoryProgramStatus = 0;
__IO uint32_t data32 = 0;
uint8_t str[16];
uint64_t buf[250];


uint32_t Address1=FLASH_USER_END_ADDR-8;   //(ADDR_FLASH_PAGE_6/2);
//uint32_t Address2=FLASH_USER_END_ADDR;
FLASH_EraseInitTypeDef EraseInitStruct;

static uint32_t GetPage(uint32_t );
static uint32_t GetBank(uint32_t );



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_ICACHE_Init(void);

//void LED_Thread1(void *argument);
//void LED_Thread2(void *argument);

/* USER CODE BEGIN PFP */
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
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

//  BSP_LED_Init(LED9);
  //BSP_LED_Init(LED10);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_ICACHE_Init();
  /* USER CODE BEGIN 2 */
  MX_GPIO_Init();
  MX_I2C1_Init();
  ssd1306_Init();

  //memset(str, data ,16);

  /* Initialize test status */
  //MemoryProgramStatus = PASSED;

  HAL_FLASH_Unlock();
 // HAL_FLASH_OB_Unlock();
  FirstPage = GetPage(FLASH_USER_START_ADDR);
  NbOfPages = GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;
  BankNumber = GetBank(FLASH_USER_START_ADDR);

/*
   OptionsBytesStruct.WRPArea  = OB_WRPAREA_BANK1_AREAA;
   OptionsBytesStruct2.WRPArea = OB_WRPAREA_BANK1_AREAB;*/

#ifdef FLASH_DOUBLEWORD_TYPEPROGRAM
  /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = BankNumber;
    EraseInitStruct.Page        = FirstPage;
    EraseInitStruct.NbPages     = NbOfPages;


    /*  Erasing a page  */
    HAL_FLASH_Unlock();
     if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)!=HAL_OK)
     {
    	printf("error");
     }
    HAL_FLASH_Lock();


    /*  Writing some data on the page  */
    HAL_FLASH_Unlock();
     for(i=0;i<250;i++)
      {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD ,Address, data1);     //FLASH_TYPEPROGRAM_DOUBLEWORD
         Address=Address+8;
       }
    HAL_FLASH_Lock();


    /*  Reading data from the page  */
    Address1=FLASH_USER_START_ADDR ;
    HAL_FLASH_Unlock();
    for(i=0;i<250;i++)
    {
      rdata=*( uint64_t *)Address1;
      buf[i]=rdata;
      Address1=Address1+8;
    }
    HAL_FLASH_Lock();


    /*  Again erasing the whole page  */
     HAL_FLASH_Unlock();
     if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)!=HAL_OK)
       {
    	printf("error");
       }
     HAL_FLASH_Lock();


     /*  Modifying data in buffer at particular index   */
     buf[16]=data;


     /*  Writing buffer data at the page with modified data  */
     Address=FLASH_USER_START_ADDR ;
     HAL_FLASH_Unlock();
     for(i=0;i<250;i++)
     {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD ,Address, buf[i]);     //FLASH_TYPEPROGRAM_DOUBLEWORD
      Address=Address+8;
     }
     HAL_FLASH_Lock();


  /*Address1=FLASH_USER_START_ADDR ;
  HAL_FLASH_Unlock();
  //for(i=0;i<1;i++)
    //{
    rdata=(__IO uint8_t *)Address1;
    ssd1306_Data(data);
    Address1=Address1+8;
    HAL_Delay(500);
    //}
  HAL_FLASH_Lock();*/
#endif

  /* USER CODE END 2 */

  /* Init scheduler */
  //osKernelInitialize();


  /* Start scheduler */
 // osKernelStart();


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */


    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
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

/* USER CODE BEGIN 4 */

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
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr)
{
  return FLASH_BANK_1;
}

/* USER CODE END 4 */





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
   //USER CODE BEGIN Error_Handler_Debug
   //User can add his own implementation to report the HAL error return state
  while(1)
  {
  }
  // USER CODE END Error_Handler_Debug
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
