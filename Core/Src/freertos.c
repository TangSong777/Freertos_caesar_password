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
#include "KEY.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  OUTPUT = 0,
  EZINPUT = 1,
  INPUT = 2
} PUT_STATE; // 长按按键切换状濿

typedef enum
{
  mode1 = 0,
  mode2 = 1,
  mode3 = 2
} LED_STATE;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define Bright GPIO_PIN_SET
#define Dark GPIO_PIN_RESET
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
PUT_STATE PutState = OUTPUT; // 默认为输出信号模�????
LED_STATE LedState = mode1;
uint8_t Key_flag = 0;      // 按键标志定义
uint32_t Bright_time = 0;  // 代表亮起的单位时�????
uint32_t Dark_time = 0;    // 代表熄灭的单位时�????
uint8_t Start_ezinput = 0; // 定义弿始接收标忿
uint8_t Start_input = 0;   // 定义弿始接收标忿
uint8_t Start_flash = 0;   // 定义弿始转换标忿
uint8_t Space_num = 0;     // 定义字符串中的空格数

uint8_t Receive_morse[100] = {0}; // 定义数组储存摩尔斯密�????
uint8_t Receive_str[200] = {0};   // 定义数组储存字符�????
uint8_t Receive_morse_len = 0;    // 定义摩尔斯密码长�????
uint8_t Receive_str_len = 0;      // 定义字符串长�????

uint8_t Transmit_morse[100] = {0};
uint8_t Transmit_morse_len = 0;

uint8_t Start_receive = 0;
uint8_t Start_transmit = 0;
uint8_t Process = 0;
uint8_t T = 0; // 定义T
/* USER CODE END Variables */
/* Definitions for ProcessTask */
osThreadId_t ProcessTaskHandle;
const osThreadAttr_t ProcessTask_attributes = {
    .name = "ProcessTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for ReceiveDataTask */
osThreadId_t ReceiveDataTaskHandle;
const osThreadAttr_t ReceiveDataTask_attributes = {
    .name = "ReceiveDataTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
};
/* Definitions for ReceiveTask */
osThreadId_t ReceiveTaskHandle;
const osThreadAttr_t ReceiveTask_attributes = {
    .name = "ReceiveTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for TransmitTask */
osThreadId_t TransmitTaskHandle;
const osThreadAttr_t TransmitTask_attributes = {
    .name = "TransmitTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for KeyScanTask */
osThreadId_t KeyScanTaskHandle;
const osThreadAttr_t KeyScanTask_attributes = {
    .name = "KeyScanTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for DetectTimer */
osTimerId_t DetectTimerHandle;
const osTimerAttr_t DetectTimer_attributes = {
    .name = "DetectTimer"};
/* Definitions for Usart */
osSemaphoreId_t UsartHandle;
const osSemaphoreAttr_t Usart_attributes = {
    .name = "Usart"};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Delay_break(uint32_t ms)
{
  for (uint16_t i = 0; i < ms / 10; i++)
  {
    if (Key_flag)
      break;
    else
      osDelay(10);
  }
}
void Transform_password(uint8_t str[], uint8_t str_len, uint8_t t)
{
  for (int i = 0; i < str_len; i++)
  {
    if (str[i] >= 'a' && str[i] <= 'z')
    {
      str[i] = (str[i] - 'a' - t + 26) % 26 + 'a';
    }
  };
}

int Judge_str(uint8_t str1[], const char str2[], int len)
{
  for (int i = 0; i < len; i++)
  {
    if (str1[i] == str2[i])
      continue;
    else
      return 0;
  }
  return 1;
}

void Morse_to_str(uint8_t receive_morse[], uint8_t receive_str[], uint8_t receive_morse_len, uint8_t receive_str_len)
{
  switch (receive_morse_len)
  {
  case 1:
  {
    if (Judge_str(receive_morse, ".", receive_morse_len))
      receive_str[receive_str_len] = 'e';
    if (Judge_str(receive_morse, "-", receive_morse_len))
      receive_str[receive_str_len] = 't';
    break;
  }
  case 2:
  {
    if (Judge_str(receive_morse, ".-", receive_morse_len))
      receive_str[receive_str_len] = 'a';
    if (Judge_str(receive_morse, "..", receive_morse_len))
      receive_str[receive_str_len] = 'i';
    if (Judge_str(receive_morse, "--", receive_morse_len))
      receive_str[receive_str_len] = 'm';
    if (Judge_str(receive_morse, "-.", receive_morse_len))
      receive_str[receive_str_len] = 'n';
    break;
  }
  case 3:
  {
    if (Judge_str(receive_morse, "-..", receive_morse_len))
      receive_str[receive_str_len] = 'd';
    if (Judge_str(receive_morse, "--.", receive_morse_len))
      receive_str[receive_str_len] = 'g';
    if (Judge_str(receive_morse, "-.-", receive_morse_len))
      receive_str[receive_str_len] = 'k';
    if (Judge_str(receive_morse, "---", receive_morse_len))
      receive_str[receive_str_len] = 'o';
    if (Judge_str(receive_morse, ".-.", receive_morse_len))
      receive_str[receive_str_len] = 'r';
    if (Judge_str(receive_morse, "...", receive_morse_len))
      receive_str[receive_str_len] = 's';
    if (Judge_str(receive_morse, "..-", receive_morse_len))
      receive_str[receive_str_len] = 'u';
    if (Judge_str(receive_morse, ".--", receive_morse_len))
      receive_str[receive_str_len] = 'w';
    break;
  }
  case 4:
  {
    if (Judge_str(receive_morse, "-...", receive_morse_len))
      receive_str[receive_str_len] = 'b';
    if (Judge_str(receive_morse, "-.-.", receive_morse_len))
      receive_str[receive_str_len] = 'c';
    if (Judge_str(receive_morse, "..-.", receive_morse_len))
      receive_str[receive_str_len] = 'f';
    if (Judge_str(receive_morse, "....", receive_morse_len))
      receive_str[receive_str_len] = 'h';
    if (Judge_str(receive_morse, ".---", receive_morse_len))
      receive_str[receive_str_len] = 'j';
    if (Judge_str(receive_morse, ".-..", receive_morse_len))
      receive_str[receive_str_len] = 'l';
    if (Judge_str(receive_morse, ".--.", receive_morse_len))
      receive_str[receive_str_len] = 'p';
    if (Judge_str(receive_morse, "--.-", receive_morse_len))
      receive_str[receive_str_len] = 'q';
    if (Judge_str(receive_morse, "...-", receive_morse_len))
      receive_str[receive_str_len] = 'v';
    if (Judge_str(receive_morse, "-..-", receive_morse_len))
      receive_str[receive_str_len] = 'x';
    if (Judge_str(receive_morse, "-.--", receive_morse_len))
      receive_str[receive_str_len] = 'y';
    if (Judge_str(receive_morse, "--..", receive_morse_len))
      receive_str[receive_str_len] = 'z';
    break;
  }
  }
}
void Assign_str(uint8_t morse[], const uint8_t str[], uint8_t len)
{
  for (int i = 0; i < len; i++)
  {
    morse[Transmit_morse_len++] = str[i];
  }
  morse[Transmit_morse_len++] = 'l';
}
void Str_to_morse(uint8_t transmit_str[], uint8_t transmit_morse[], uint8_t transmit_str_len)
{
  for (int i = 0; i < transmit_str_len; i++)
  {
    if (transmit_str[i] != ' ')
    {
      switch (transmit_str[i])
      {
      case 'a':
        Assign_str(transmit_morse, ".-", 2);
        break;
      case 'b':
        Assign_str(transmit_morse, "-...", 4);
        break;
      case 'c':
        Assign_str(transmit_morse, "-.-.", 4);
        break;
      case 'd':
        Assign_str(transmit_morse, "-..", 3);
        break;
      case 'e':
        Assign_str(transmit_morse, ".", 1);
        break;
      case 'f':
        Assign_str(transmit_morse, "..-.", 4);
        break;
      case 'g':
        Assign_str(transmit_morse, "--.", 3);
        break;
      case 'h':
        Assign_str(transmit_morse, "....", 4);
        break;
      case 'i':
        Assign_str(transmit_morse, "..", 2);
        break;
      case 'j':
        Assign_str(transmit_morse, ".---", 4);
        break;
      case 'k':
        Assign_str(transmit_morse, "-.-", 3);
        break;
      case 'l':
        Assign_str(transmit_morse, ".-..", 4);
        break;
      case 'm':
        Assign_str(transmit_morse, "--", 2);
        break;
      case 'n':
        Assign_str(transmit_morse, "-.", 2);
        break;
      case 'o':
        Assign_str(transmit_morse, "---", 3);
        break;
      case 'p':
        Assign_str(transmit_morse, ".--.", 4);
        break;
      case 'q':
        Assign_str(transmit_morse, "--.-", 4);
        break;
      case 'r':
        Assign_str(transmit_morse, ".-.", 3);
        break;
      case 's':
        Assign_str(transmit_morse, "...", 3);
        break;
      case 't':
        Assign_str(transmit_morse, "-", 1);
        break;
      case 'u':
        Assign_str(transmit_morse, "..-", 3);
        break;
      case 'v':
        Assign_str(transmit_morse, "...-", 4);
        break;
      case 'w':
        Assign_str(transmit_morse, ".--", 3);
        break;
      case 'x':
        Assign_str(transmit_morse, "-..-", 4);
        break;
      case 'y':
        Assign_str(transmit_morse, "-.--", 4);
        break;
      case 'z':
        Assign_str(transmit_morse, "--..", 4);
        break;
      }
    }
    else
      transmit_morse[Transmit_morse_len - 1] = 'w';
  }
  transmit_morse[Transmit_morse_len - 1] = 's';
}

void Morse_to_signal(uint8_t transmit_morse[], uint8_t transmit_morse_len)
{
  for (int i = 0; i < transmit_morse_len; i++)
  {
    switch (transmit_morse[i])
    {
    case 'l':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
      osDelay(20);
      break;
    case 'w':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
      osDelay(60);
      break;
    case 's':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
      osDelay(70);
      break;
    case '.':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_SET);
      osDelay(10);
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
      osDelay(10);
      break;
    case '-':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_SET);
      osDelay(30);
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
      osDelay(10);
      break;
    }
  }
}
/* USER CODE END FunctionPrototypes */

void StartProcessTask(void *argument);
void StartReceiveDataTask(void *argument);
void StartReceiveTask(void *argument);
void StartTransmitTask(void *argument);
void StartKeyScanTask(void *argument);
void Callback1(void *argument);

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of DetectTimer */
  DetectTimerHandle = osTimerNew(Callback1, osTimerPeriodic, NULL, &DetectTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ProcessTask */
  ProcessTaskHandle = osThreadNew(StartProcessTask, NULL, &ProcessTask_attributes);

  /* creation of ReceiveDataTask */
  ReceiveDataTaskHandle = osThreadNew(StartReceiveDataTask, NULL, &ReceiveDataTask_attributes);

  /* creation of ReceiveTask */
  ReceiveTaskHandle = osThreadNew(StartReceiveTask, NULL, &ReceiveTask_attributes);

  /* creation of TransmitTask */
  TransmitTaskHandle = osThreadNew(StartTransmitTask, NULL, &TransmitTask_attributes);

  /* creation of KeyScanTask */
  KeyScanTaskHandle = osThreadNew(StartKeyScanTask, NULL, &KeyScanTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartProcessTask */
/**
 * @brief  Function implementing the ProcessTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartProcessTask */
void StartProcessTask(void *argument)
{
  /* USER CODE BEGIN StartProcessTask */
  HAL_UART_Receive_IT(&huart2, (uint8_t *)RxTemp, 1);
  Key_Init(KEY0, KEY_GPIO_Port, KEY_Pin, PULL_UP);
  osTimerStart(DetectTimerHandle, 10); // 10代表回调函数回调周期
  /* Infinite loop */
  while (1)
  {
    switch (Process)
    {
    case 1:
      if (Bright_time == 1)
        Receive_morse[Receive_morse_len++] = '.';
      else if (Bright_time == 3)
        Receive_morse[Receive_morse_len++] = '-';
      Bright_time = 0;
      break;
    case 3:
      printf("Morse:");
      for (int i = 0; i < Receive_morse_len; i++)
        printf("%c", Receive_morse[i]);
      printf("\r\n");
      Morse_to_str(Receive_morse, Receive_str, Receive_morse_len, Receive_str_len++);
      memset(Receive_morse, 0, 50);
      Receive_morse_len = 0;
      for (int i = 0; i < Receive_str_len; i++)
        printf("%c", Receive_str[i]);
      printf("\r\n");
      Process = 0;
      break;
    case 7:
      Receive_str[Receive_str_len++] = ' ';
      Space_num++;
      break;
    case 8:
      Receive_str[--Receive_str_len] = 0;
      Space_num--;
      T = (Receive_str_len - Space_num) % 7;
      printf("Str_len:%d\r\n", Receive_str_len);
      printf("Space:%d\r\n", Space_num);
      printf("T:%d\r\n", T);
      Transform_password(Receive_str, Receive_str_len, T);
      printf("EndStr:");
      for (int i = 0; i < Receive_str_len; i++)
        printf("%c", Receive_str[i]);
      printf(".\r\n\r\n");
      memset(Receive_str, 0, Receive_str_len);
      Receive_str_len = 0;
      Space_num = 0;
      Dark_time = 0;
      Start_input = 0;
      Start_receive = 0;
      Process = 0;
      break;
    }
    osDelay(1);
  }

  /* USER CODE END StartProcessTask */
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
    osSemaphoreAcquire(UsartHandle, osWaitForever); // 等待二�?�信号量
    if (RxFlag == 1)                                // 数据接收完成
    {
      for (int i = 0; i < RxCounter; i++) // 打印接收数组存储的内�????
        printf("%c", RxBuffer[i]);
      printf("\r\n"); // 打印完成换行
      RxFlag = 0;     // 接收标志清零
      Str_to_morse(RxBuffer, Transmit_morse, RxCounter);
      memset(RxBuffer, 0, 2048); // 清空接收数组
      RxCounter = 0;
    }
    printf("Morse:");
    for (int i = 0; i < Transmit_morse_len; i++) // 打印接收数组存储的内�????
      printf("%c", Transmit_morse[i]);
    printf("\r\n");
    Start_receive = 1;
    Start_transmit = 1;
  }
  /* USER CODE END StartReceiveDataTask */
}

/* USER CODE BEGIN Header_StartReceiveTask */
/**
 * @brief Function implementing the ReceiveTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReceiveTask */
void StartReceiveTask(void *argument)
{
  /* USER CODE BEGIN StartReceiveTask */
  /* Infinite loop */
  while (1)
  {
    osDelay(10);
  }
  /* USER CODE END StartReceiveTask */
}

/* USER CODE BEGIN Header_StartTransmitTask */
/**
 * @brief Function implementing the TransmitTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTransmitTask */
void StartTransmitTask(void *argument)
{
  /* USER CODE BEGIN StartTransmitTask */
  /* Infinite loop */
  while (1)
  {
    switch (PutState)
    {
    case OUTPUT:
    case EZINPUT:
      if (!Key_flag)
      {
        switch (LedState)
        {
        case mode1:
        {
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_SET);
          Delay_break(1000);
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
          Delay_break(1000);
          break;
        }
        case mode2:
        {
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_SET);
          Delay_break(2000);
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
          Delay_break(2000);
          break;
        }
        case mode3:
        {
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_SET);
          Delay_break(3000);
          HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
          Delay_break(3000);
          break;
        }
        }
      }
      break;
    case INPUT:
      if (Start_transmit)
      {
        printf("Transmit!\r\n");
        osDelay(20);
        Morse_to_signal(Transmit_morse, Transmit_morse_len);
        memset(Transmit_morse, 0, Transmit_morse_len);
        Transmit_morse_len = 0;
        Start_transmit = 0;
      }
      break;
    default:
      break;
    }
  }
  vTaskDelete(NULL);

  /* USER CODE END StartTransmitTask */
}

/* USER CODE BEGIN Header_StartKeyScanTask */
/**
 * @brief Function implementing the KeyScanTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartKeyScanTask */
void StartKeyScanTask(void *argument)
{
  /* USER CODE BEGIN StartKeyScanTask */
  /* Infinite loop */
  while (1)
  {
    osDelay(10);
    Key_pressscan(KEY0, &Key_flag);
    if (Key_flag == 1)
    {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
      Bright_time = 0;
      Dark_time = 0;
      Start_ezinput = 0;
      Start_input = 0;
      osDelay(100);
      if (LedState < mode3)
      {
        LedState++;
        printf("Ledmode:%d.\r\n", LedState + 1);
      }
      else
      {
        LedState = mode1;
        printf("Ledmode:1.\r\n");
      }
      Key_flag = 0;
    }
    if (Key_flag == 3)
    {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Light_output_GPIO_Port, Light_output_Pin, GPIO_PIN_RESET);
      Bright_time = 0;
      Dark_time = 0;
      Start_ezinput = 0;
      Start_input = 0;
      if (PutState < INPUT)
        PutState++;
      else
        PutState = OUTPUT;
      switch (PutState)
      {
      case OUTPUT:
      {
        printf("OUTPUT.\r\n");
        break;
      }
      case EZINPUT:
      {
        printf("EZINPUT.\r\n");
        break;
      }
      case INPUT:
      {
        printf("INPUT.\r\n");
        break;
      }
      default:
        break;
      }
      Key_flag = 0;
    }
  }
  vTaskDelete(NULL);
  /* USER CODE END StartKeyScanTask */
}

/* Callback1 function */
void Callback1(void *argument)
{
  /* USER CODE BEGIN Callback01 */
  switch (PutState)
  {
  case INPUT:
  {
    if (Start_receive)
    {
      if (HAL_GPIO_ReadPin(Light_input_GPIO_Port, Light_input_Pin) == Bright)
      {
        if (!Start_input)
        {
          Start_input = 1;
          printf("Receive!\r\n");
        }
        Bright_time++;
        Dark_time = 0;
      }
      else if (HAL_GPIO_ReadPin(Light_input_GPIO_Port, Light_input_Pin) == Dark && Start_input)
      {
        Dark_time++;
        switch (Dark_time)
        {
        case 1:
        {
          if (!Process)
            Process = 1;
          break;
        }
        case 3:
        {
          if (!Process)
            Process = 3;
          break;
        }
        case 7:
        {
          if (!Process)
            Process = 7;
          break;
        }
        default:
          break;
        }
        if (Dark_time > 7 && !Process)
        {
          Process = 8;
        }
      }
    }
  }
  default:
    break;
  }
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
