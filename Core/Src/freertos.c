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
#include "usart.h"
#include "stdio.h"
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for ReceiveDataTask */
osThreadId_t ReceiveDataTaskHandle;
const osThreadAttr_t ReceiveDataTask_attributes = {
    .name = "ReceiveDataTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for Usart */
osSemaphoreId_t UsartHandle;
const osSemaphoreAttr_t Usart_attributes = {
    .name = "Usart"};
/* Definitions for Signal */
osSemaphoreId_t SignalHandle;
const osSemaphoreAttr_t Signal_attributes = {
    .name = "Signal"};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartReceiveDataTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of Usart */
  UsartHandle = osSemaphoreNew(1, 0, &Usart_attributes);

  /* creation of Signal */
  SignalHandle = osSemaphoreNew(1, 0, &Signal_attributes);

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
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of ReceiveDataTask */
  ReceiveDataTaskHandle = osThreadNew(StartReceiveDataTask, NULL, &ReceiveDataTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  HAL_UART_Receive_IT(&huart2, (uint8_t *)RxTemp, 1);
  /* Infinite loop */
  for (;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartReceiveDataTask */
/**
 * @brief Function implementing the ReceiveDataTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReceiveDataTask */
void StartReceiveDataTask(void *argument)
{
  /* USER CODE BEGIN StartReceiveDataTask */
  /* Infinite loop */
  while (1)
  {
    osSemaphoreAcquire(UsartHandle, 100); // 等待二值信号量
    if (RxFlag == 1)                      // 数据接收完成
    {
      for (int i = 0; i < RxCounter; i++) // 打印接收数组存储的内容
        printf("%c", RxBuffer[i]);
      printf("\r\n");            // 打印完成换行
      RxFlag = 0;                // 接收标志清零
      RxCounter = 0;             // 接收计数清零
      memset(RxBuffer, 0, 2048); // 清空接收数组
    }
    // Str_to_morse();
    // osSemaphoreRelease(SignalHandle);
  }
  /* USER CODE END StartReceiveDataTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
