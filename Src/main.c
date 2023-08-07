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
#include "FreeRTOS.h"
#include "timers.h"

#include "FreeRTOS.h"
#include "stm32l5xx_hal_uart.h"
#include "stm32l5xx_hal_uart_ex.h"
#include "timers.h"
#include "queue.h"



/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"


//#define RX_BUFFER_SIZE 100
#define QUEUE_LENGTH 100
#define Queue_Using_API

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 100 )
#define count 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/


/* USER CODE BEGIN PV */

TimerHandle_t xAutoReloadTimer, xOneShotTimer;
__IO uint32_t OsStatus = 0;
TaskHandle_t task1_handle, task2_handle;
//QueueHandle_t xQueue;
QueueHandle_t xPointerQueue;

UART_HandleTypeDef huart1;
HAL_StatusTypeDef error;
QueueHandle_t xPointerQueue;


//volatile int front=-1;
//volatile int rear=-1;

uint8_t msg[] = "Hi, Welcome to UART demo!!\r\n";
//uint8_t rxBuffer[RX_BUFFER_SIZE]={0};
//uint8_t data;
//volatile uint8_t flag=0;

/*volatile uint32_t rxIndex = 0;
volatile uint8_t rxComplete = 0;*/
TaskHandle_t task1_handle;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_ICACHE_Init(void);

//void LED_Thread1(void *argument);
//void LED_Thread2(void *argument);
static void prvAutoReloadTimerCallback( TimerHandle_t xTimer );

static void MX_USART1_UART_Init(void);
static void EXTI13_IRQHandler_Config(void);
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
//void insert(char data);
/* USER CODE BEGIN PFP */
static void prvTimerCallback( TimerHandle_t xTimer );

//static void prvOneShotTimerCallback( TimerHandle_t xTimer );


/* USER CODE BEGIN PFP */
static void vSenderTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );

/*
 Define an enumerated type used to identify the source of the data.
typedef enum
{
 eSender1,
 eSender2
} DataSource_t;

 Define the structure type that will be passed on the queue.
typedef struct
{
 uint8_t ucValue;
 DataSource_t eDataSource;
} Data_t;

 Declare two variables of type Data_t that will be passed on the queue.
static const Data_t xStructsToSend[ 2 ] ={{ 100, eSender1 },  Used by Sender1.
 { 200, eSender2 }  Used by Sender2. };
*/


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


/**
  * @brief  The application entry point.
  * @retval int
  */

const TickType_t xHealthyTimerPeriod = pdMS_TO_TICKS( 1000 );
//const TickType_t xErrorTimerPeriod = pdMS_TO_TICKS( 200 );


//#define mainONE_SHOT_TIMER_PERIOD pdMS_TO_TICKS( 3333 )
//#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 1000 )

//#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 100 )

int main(void)
{
  /* USER CODE BEGIN 1 */

	BaseType_t status , status1 ,status2;

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
  //BSP_LED_Init(LED9);
  //BSP_LED_Init(LED10);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  MX_ICACHE_Init();

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
  EXTI13_IRQHandler_Config();

  /* Init scheduler */
  osKernelInitialize();


  BaseType_t xTimer1Started, xTimer2Started;

 // xTaskCreate(LED_Thread1, "LED Task", 1024, NULL, 1, &task1_handle);

 // xOneShotTimer = xTimerCreate("OneShot",mainONE_SHOT_TIMER_PERIOD,pdFALSE,0,prvTimerCallback );
  xAutoReloadTimer = xTimerCreate("AutoReload",xHealthyTimerPeriod,pdTRUE,0,prvTimerCallback);

  if(  xAutoReloadTimer != NULL  )
   {
    printf("timers created\n");
   //xTimer1Started = xTimerStart( xOneShotTimer, 0 );
   xTimer2Started = xTimerStart( xAutoReloadTimer, 0 );

     if( xTimer2Started == pdPASS  )
      {
    	 /* Start the scheduler. */
    	 vTaskStartScheduler();
      }

  /* The queue is created to hold a maximum of 5 values, each of which is
   large enough to hold a variable of type Data_t ). */
   //xQueue = xQueueCreate( 3, sizeof( Data_t )) ;
   xPointerQueue = xQueueCreate( 5, sizeof( char * ) );

   if( xPointerQueue != NULL )
   {
   /* Create two instances of the task that will send to the queue. The
 parameter is used to pass the structure that the task will write to the
 queue, so one task will continuously send xStructsToSend[ 0 ] to the queue
 while the other task will continuously send xStructsToSend[ 1 ]. */
	   status = xTaskCreate(vSenderTask, "Task1", 500, NULL, 2, &task1_handle);
	 //  status1 = xTaskCreate(vSenderTask, "Task2", 500, &( xStructsToSend[ 1 ] ), 2, &task2_handle);
	   configASSERT(status == pdPASS);
	 // configASSERT(status1 == pdPASS);

   /* Create the task that will read from the queue. The task is created with
   priority 2, so above the priority of the sender tasks. */
   status2 = xTaskCreate( vReceiverTask, "Task3", 500, NULL, 1, NULL );
   configASSERT(status2 == pdPASS);
   /* Start the scheduler so the created tasks start executing. */
   vTaskStartScheduler();
   }
   else
   {
   /* The queue could not be created. */
	   printf("Queue not created\n");
   }

  /* Start scheduler */
 // osKernelStart();

    //BaseType_t status;

    //const UBaseType_t ulPeriodicTaskPriority = configTIMER_TASK_PRIORITY - 1;

    //status = xTaskCreate( vPeriodicTask, "Task1", 500, NULL, ulPeriodicTaskPriority, NULL);
    //configASSERT(status == pdPASS);

  TimerHandle_t xAutoReloadTimer = xTimerCreate( "AutoReload", AUTO_RELOAD_TIMER_PERIOD, pdTRUE,0,  prvAutoReloadTimerCallback );
    	 /* Check the software timers were created. */
    	 if(  xAutoReloadTimer != NULL  )
    	 {
    	 /* Start the software timers, using a block time of 0 (no block time). The scheduler has
    	 not been started yet so any block time specified here would be ignored anyway. */
    	xTimerStartFromISR( xAutoReloadTimer, 0 );
    	 }

  /* Start scheduler */
  vTaskStartScheduler();
    EXTI13_IRQHandler_Config();

    MX_USART1_UART_Init();
    USART1->CR1 |= USART_CR1_RXNEIE;
    HAL_UART_Transmit(&huart1, msg, sizeof(msg), 1000);
    while(!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR='A';

    BaseType_t status;
    xPointerQueue = xQueueCreate( QUEUE_LENGTH, sizeof( char * ) );
    if( xPointerQueue != NULL )
       {

    	   status = xTaskCreate(vSenderTask, "Task1", 500, NULL, 0, &task1_handle);
    	   configASSERT(status == pdPASS);
       vTaskStartScheduler();
       }
       else
       {
       /* The queue could not be created. */
    	   printf("Queue not created\n");
       }

  /* Start scheduler */
  //osKernelStart();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   //HAL_UART_Receive_IT(&huart1, &data, 1);

  while (1)
  {
    /* USER CODE END WHILE */
	  /* USER CODE BEGIN 3 */
#ifdef Single_Character_Transfer
if(flag==1)
{
   while(!(USART1->ISR & USART_ISR_TXE));
   {
	   USART1->TDR=data;
	              flag=0;
   }
}


	   /* if(rear>-1 && front<=rear)
	    {
	      //HAL_UART_Transmit(&huart1, &rxBuffer[f],1, 1000);
		  while(!(USART1->ISR & USART_ISR_TXE));
		     {
		  	   USART1->TDR=rxBuffer[front];
		  	   front++;
		     }

	    }*/
#endif

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

/* USER CODE END 4 */


/* USER CODE BEGIN Header_LED_Thread1 */

/*void LED_Thread1(void *argument)
{
   USER CODE BEGIN 5
	printf("Task1 is running\n");
	  while(1)
    {
      BSP_LED_Toggle(LED9);
      HAL_Delay(200);

    }
	  osDelay(1000);
   USER CODE END 5
}*/

static void prvTimerCallback( TimerHandle_t xTimer )
{

/*
@brief sending data to queue function
@param none
@RetVal none
*/
/*SenderTask_Handler   */
 void vSenderTask( void *pvParameters )
{
	 char *pcStringToSend;
	 const size_t xMaxStringLength = 50;
	 char buffer[50];
	 BaseType_t xStatus, xStringNumber = 0;
	  for( ;; )
	  {

	 // pcStringToSend = ( char * ) pvPortMalloc( xMaxStringLength );
	  pcStringToSend = ( char * ) buffer;

	  snprintf( pcStringToSend, xMaxStringLength, " Hello\r\n");//, xStringNumber );
	  printf("String in sendertask:  %s\n",pcStringToSend);
	  printf("String number: %d \n",xStringNumber);
	  /* Increment the counter so the string is different on each iteration of this task. */
	  xStringNumber++;

	  xStatus= xQueueSend( xPointerQueue,&pcStringToSend, portMAX_DELAY );

	  if( xStatus != pdPASS )
	   {
		  /* The send operation could not complete because the queue was full -this must be an error as the queue should never contain more than
	   	   one item! */
		  printf( "Could not send to the queue.\r\n" );
	   }
	   else
	   {
		   printf( "Send pass\r\n" );
		   vTaskDelay(pdMS_TO_TICKS(100));
	   }
	  }
 }


//static BaseType_t xErrorDetected = pdFALSE;
printf("timer expired \n");


TickType_t TimeNow;
 /* Obtain the current tick count. */
 TimeNow = xTaskGetTickCount();
 /* Output a string to show the time at which the callback was executed. */
 printf( "One-shot timer callback executing %d \n", TimeNow );
vTaskDelay(5000);
xTimerReset(xTimer, 0);

/*if( xErrorDetected == pdFALSE )
	{

	//if( CheckTasksAreRunningWithoutError() == pdFAIL )
		if(eTaskGetState(task1_handle) != eRunning )
		{

			xTimerChangePeriod( xTimer, xErrorTimerPeriod, 0 );  Do not block when sending this command.
		}
		 Latch that an error has already been detected.
		xErrorDetected = pdTRUE;
	}
BSP_LED_Toggle(LED9);*/


 /*

 @brief receiving data from queue function
 @param none
 @RetVal none
 */
/* ReceiverTask_Handler*/
static void vReceiverTask( void *pvParameters )
{
	int num;
	char *pcReceivedString;
	//BaseType_t xStatus;
	 for( ;; )
	 {
		 num=uxQueueMessagesWaiting( xPointerQueue );
         //for checking queue is full or not
		 if( num == 5 )
		 	 {
			 printf( "Queue is full!\r\n" );
		 	 }
		 else if(num == 0)
		 {
			 printf( "Queue is empty!\r\n" );
		 }


	 /* Receive the address of a buffer. */
	 xQueueReceive( xPointerQueue,&pcReceivedString, portMAX_DELAY );
	 /* The buffer holds a string, print it out. */

	 printf( "String in receiver task: %s \n",pcReceivedString );
	 vTaskDelay(pdMS_TO_TICKS(100));
	 /* The buffer is not required any more - release it so it can be freed, or re-used. */
	 //vPortFree( pcReceivedString );
	 }
/**
  * @brief interrupt and uart GPIO Initialization Function
  * @param None
  * @retval None
  */
static void EXTI13_IRQHandler_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
      PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
      if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
      {
        Error_Handler();
      }
  /* Enable GPIOC clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Configure PC.13 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = BUTTON_USER_PIN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PA.9 pin as UART TX */
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_9;
  GPIO_InitStructure.Alternate =GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PA.10 pin as UART RX */
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  GPIO_InitStructure.Alternate =GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable and set line 13 Interrupt to the lowest priority */
  //HAL_NVIC_SetPriority(EXTI13_IRQn, 2, 0);
  //HAL_NVIC_EnableIRQ(EXTI13_IRQn);
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

}

/**
  * @brief UART Initialization Function
  * @param None
  * @retval None
  */

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}




/**
  * @brief  USART1 IRQ Handler.
  * This function serves as the interrupt handler for USART1. It is responsible
  * for handling incoming data in the USART1 receive buffer and sending it to
  * a FreeRTOS queue for processing in other task.
  * @note   This function handles the interrupts generated by USART1 peripheral.
  *         It reads the received data from the USART1 receive data register (RDR)
  * @retval None
  */

void USART1_IRQHandler(void) // Change IRQ handler name as per your UART peripheral
{
#ifdef Queue_Using_Register
//    HAL_UART_IRQHandler(&huart1);
	if(front==-1)
	{
		front=0;
	}
    if(USART1->ISR & USART_ISR_RXNE)
    {
    	rear++;
    	rxBuffer[rear]=USART1->RDR;
    }
#endif


#ifdef Queue_Using_API
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if (USART1->ISR & USART_ISR_RXNE)
		{
			xQueueSendFromISR(xPointerQueue, (uint8_t*)&USART1->RDR, &xHigherPriorityTaskWoken);

		}

		if (xHigherPriorityTaskWoken)
			{
				// Actual macro used here is port specific.
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
}
#endif


/**
  * @brief  Sender Task.
  * This task is responsible for sending data from a queue to the USART1 interface.
  * It continuously checks for available data in the 'xPointerQueue' and transmits it
  * via USART1 if there is data available.
  * @para pvParameters A pointer to task parameters (not used in this task).
  * @retval None
  */
void vSenderTask( void *pvParameters )
{
	while (1)
		{
			char data = 0;
			if (xQueueReceive(xPointerQueue, &data, 0) == pdTRUE)
			{
				while (!(USART1->ISR & USART_ISR_TXE));
				USART1->TDR = data;
			}
		}
}



static void EXTI13_IRQHandler_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;


  /* Enable GPIOC clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure PC.13 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;


  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = BUTTON_USER_PIN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);


  /* Enable and set line 13 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI13_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI13_IRQn);
}



/**
  * @brief Autoreload timer callback function, here led toggles only on long press of 1s
  * @param xTimer: Timer handle
  * @retval None
  */
static void prvAutoReloadTimerCallback( TimerHandle_t xTimer )
{
TickType_t xTimeNow;
//TickType_t buttonPressTime = 0;
static int count1=0;
    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();
    /* Output a string to show the time at which the callback was executed. */
 printf( "Auto-reload timer callback executing %d\n", xTimeNow );

  if(HAL_GPIO_ReadPin(BUTTON_USER_GPIO_PORT, BUTTON_USER_PIN)==1)
  {
	 count1++;
	 if(count1==count)
         HAL_GPIO_TogglePin(LED10_GPIO_PORT, LED10_PIN);
  }
  else
	 count1=0;

printf("count %d \n",count1);
 }










/*
/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
/*void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   //USER CODE BEGIN Callback 0

 //  USER CODE END Callback 0
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
 //  USER CODE BEGIN Callback 1

  // USER CODE END Callback 1
}*/

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
