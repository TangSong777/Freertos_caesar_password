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
PUT_STATE PutState = OUTPUT; // 默认为输出信号模�?
LED_STATE LedState = mode1;
uint8_t Key_flag = 0;      // 按键标志定义
uint32_t Bright_time = 0;  // 代表亮起的单位时�?
uint32_t Dark_time = 0;    // 代表熄灭的单位时�?
uint8_t Start_ezinput = 0; // 定义弿始接收标忿
uint8_t Start_input = 0;   // 定义弿始接收标忿
uint8_t Start_flash = 0;   // 定义弿始转换标忿
uint8_t Morse_len = 0;     // 定义摩尔斯密码长�?
uint8_t Str_len = 0;       // 定义字符串长�?
uint8_t Space_num = 0;     // 定义字符串中的空格数
uint8_t T = 0;             // 定义T
uint8_t Morse[50] = {0};   // 定义数组储存摩尔斯密�?
uint8_t Str[200] = {0};    // 定义数组储存字符�?
uint8_t Signal_morse[100] = {0};
uint8_t Signal_morse_len = 0;
uint8_t test = 1;
/* USER CODE END Variables */
/* Definitions for DefaultTask */
osThreadId_t DefaultTaskHandle;
const osThreadAttr_t DefaultTask_attributes = {
  .name = "DefaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ReceiveDataTask */
osThreadId_t ReceiveDataTaskHandle;
const osThreadAttr_t ReceiveDataTask_attributes = {
  .name = "ReceiveDataTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for TimeDetectTask */
osThreadId_t TimeDetectTaskHandle;
const osThreadAttr_t TimeDetectTask_attributes = {
  .name = "TimeDetectTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SignalTask */
osThreadId_t SignalTaskHandle;
const osThreadAttr_t SignalTask_attributes = {
  .name = "SignalTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KeyScanTask */
osThreadId_t KeyScanTaskHandle;
const osThreadAttr_t KeyScanTask_attributes = {
  .name = "KeyScanTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Usart */
osSemaphoreId_t UsartHandle;
const osSemaphoreAttr_t Usart_attributes = {
  .name = "Usart"
};
/* Definitions for Signal */
osSemaphoreId_t SignalHandle;
const osSemaphoreAttr_t Signal_attributes = {
  .name = "Signal"
};
/* Definitions for Detect */
osSemaphoreId_t DetectHandle;
const osSemaphoreAttr_t Detect_attributes = {
  .name = "Detect"
};

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

int Judge(uint8_t str1[], const char str2[], int len)
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

void Morse_to_str(uint8_t morse[], uint8_t str[], uint8_t morse_len, uint8_t str_len)
{
  switch (morse_len)
  {
  case 1:
  {
    if (Judge(morse, ".", morse_len))
      str[str_len] = 'e';
    if (Judge(morse, "-", morse_len))
      str[str_len] = 't';
    break;
  }
  case 2:
  {
    if (Judge(morse, ".-", morse_len))
      str[str_len] = 'a';
    if (Judge(morse, "..", morse_len))
      str[str_len] = 'i';
    if (Judge(morse, "--", morse_len))
      str[str_len] = 'm';
    if (Judge(morse, "-.", morse_len))
      str[str_len] = 'n';
    break;
  }
  case 3:
  {
    if (Judge(morse, "-..", morse_len))
      str[str_len] = 'd';
    if (Judge(morse, "--.", morse_len))
      str[str_len] = 'g';
    if (Judge(morse, "-.-", morse_len))
      str[str_len] = 'k';
    if (Judge(morse, "---", morse_len))
      str[str_len] = 'o';
    if (Judge(morse, ".-.", morse_len))
      str[str_len] = 'r';
    if (Judge(morse, "...", morse_len))
      str[str_len] = 's';
    if (Judge(morse, "..-", morse_len))
      str[str_len] = 'u';
    if (Judge(morse, ".--", morse_len))
      str[str_len] = 'w';
    break;
  }
  case 4:
  {
    if (Judge(morse, "-...", morse_len))
      str[str_len] = 'b';
    if (Judge(morse, "-.-.", morse_len))
      str[str_len] = 'c';
    if (Judge(morse, "..-.", morse_len))
      str[str_len] = 'f';
    if (Judge(morse, "....", morse_len))
      str[str_len] = 'h';
    if (Judge(morse, ".---", morse_len))
      str[str_len] = 'j';
    if (Judge(morse, ".-..", morse_len))
      str[str_len] = 'l';
    if (Judge(morse, ".--.", morse_len))
      str[str_len] = 'p';
    if (Judge(morse, "--.-", morse_len))
      str[str_len] = 'q';
    if (Judge(morse, "...-", morse_len))
      str[str_len] = 'v';
    if (Judge(morse, "-..-", morse_len))
      str[str_len] = 'x';
    if (Judge(morse, "-.--", morse_len))
      str[str_len] = 'y';
    if (Judge(morse, "--..", morse_len))
      str[str_len] = 'z';
    break;
  }
  }
  memset(morse, 0, morse_len);
}
void Assign(uint8_t morse[], const uint8_t str[], uint8_t *signal_morse_len, uint8_t len)
{
  for (int i = 0; i < len; i++)
  {
    morse[*signal_morse_len++] = str[i];
  }
  morse[*signal_morse_len++] = 'l';
}
void Str_to_morse(uint8_t str[], uint8_t morse[], uint8_t str_len)
{
  for (int i = 0; i < str_len; i++)
  {
    switch (str[i])
    {
    case ' ':
      morse[Signal_morse_len] = 'w';
      break;
    case 'a':
      Assign(morse, ".-", &Signal_morse_len, 2);
      break;
    case 'b':
      Assign(morse, "-...", &Signal_morse_len, 4);
      break;
    case 'c':
      Assign(morse, "-.-.", &Signal_morse_len, 4);
      break;
    case 'd':
      Assign(morse, "-..", &Signal_morse_len, 3);
      break;
    case 'e':
      Assign(morse, ".", &Signal_morse_len, 1);
      break;
    case 'f':
      Assign(morse, "..-.", &Signal_morse_len, 4);
      break;
    case 'g':
      Assign(morse, "--.", &Signal_morse_len, 3);
      break;
    case 'h':
      Assign(morse, "....", &Signal_morse_len, 4);
      break;
    case 'i':
      Assign(morse, "..", &Signal_morse_len, 2);
      break;
    case 'j':
      Assign(morse, ".---", &Signal_morse_len, 4);
      break;
    case 'k':
      Assign(morse, "-.-", &Signal_morse_len, 3);
      break;
    case 'l':
      Assign(morse, ".-..", &Signal_morse_len, 4);
      break;
    case 'm':
      Assign(morse, "--", &Signal_morse_len, 2);
      break;
    case 'n':
      Assign(morse, "-.", &Signal_morse_len, 2);
      break;
    case 'o':
      Assign(morse, "---", &Signal_morse_len, 3);
      break;
    case 'p':
      Assign(morse, ".--.", &Signal_morse_len, 4);
      break;
    case 'q':
      Assign(morse, "--.-", &Signal_morse_len, 4);
      break;
    case 'r':
      Assign(morse, ".-.", &Signal_morse_len, 3);
      break;
    case 's':
      Assign(morse, "...", &Signal_morse_len, 3);
      break;
    case 't':
      Assign(morse, "-", &Signal_morse_len, 1);
      break;
    case 'u':
      Assign(morse, "..-", &Signal_morse_len, 3);
      break;
    case 'v':
      Assign(morse, "...-", &Signal_morse_len, 4);
      break;
    case 'w':
      Assign(morse, ".--", &Signal_morse_len, 3);
      break;
    case 'x':
      Assign(morse, "-..-", &Signal_morse_len, 4);
      break;
    case 'y':
      Assign(morse, "-.--", &Signal_morse_len, 4);
      break;
    case 'z':
      Assign(morse, "--..", &Signal_morse_len, 4);
      break;
    }
  }
}

void Morse_to_signal(uint8_t morse[], uint8_t morse_len)
{
  for (int i = 0; i < morse_len; i++)
  {
    switch (morse[i])
    {
    case 'l':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      osDelay(20);
      break;
    case 'w':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      osDelay(40);
    case '.':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      osDelay(10);
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      osDelay(10);
      break;
    case '-':
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      osDelay(30);
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      osDelay(10);
      break;
    }
  }
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartReceiveDataTask(void *argument);
void StartTimeDetectTask(void *argument);
void StartSignalTask(void *argument);
void StartKeyScanTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
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

  /* creation of Detect */
  DetectHandle = osSemaphoreNew(1, 0, &Detect_attributes);

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
  /* creation of DefaultTask */
  DefaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &DefaultTask_attributes);

  /* creation of ReceiveDataTask */
  ReceiveDataTaskHandle = osThreadNew(StartReceiveDataTask, NULL, &ReceiveDataTask_attributes);

  /* creation of TimeDetectTask */
  TimeDetectTaskHandle = osThreadNew(StartTimeDetectTask, NULL, &TimeDetectTask_attributes);

  /* creation of SignalTask */
  SignalTaskHandle = osThreadNew(StartSignalTask, NULL, &SignalTask_attributes);

  /* creation of KeyScanTask */
  KeyScanTaskHandle = osThreadNew(StartKeyScanTask, NULL, &KeyScanTask_attributes);

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
  Key_Init(KEY0, KEY_GPIO_Port, KEY_Pin, PULL_UP);
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
    osSemaphoreAcquire(UsartHandle, osWaitForever); // 等待二�?�信号量
    if (RxFlag == 1)                                // 数据接收完成
    {
      for (int i = 0; i < RxCounter; i++) // 打印接收数组存储的内�?
        printf("%c", RxBuffer[i]);
      printf("\r\n");            // 打印完成换行
      RxFlag = 0;                // 接收标志清零
      memset(RxBuffer, 0, 2048); // 清空接收数组
    }
    Str_to_morse(RxBuffer, Signal_morse, RxCounter);
    osSemaphoreRelease(SignalHandle);
  }
  /* USER CODE END StartReceiveDataTask */
}

/* USER CODE BEGIN Header_StartTimeDetectTask */
/**
* @brief Function implementing the TimeDetectTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTimeDetectTask */
void StartTimeDetectTask(void *argument)
{
  /* USER CODE BEGIN StartTimeDetectTask */
  /* Infinite loop */
  while (1)
  {
    switch (PutState)
    {
    case EZINPUT:
    {
      if (HAL_GPIO_ReadPin(Light_input_GPIO_Port, Light_input_Pin) == Bright)
      {
        Bright_time++;
        Start_ezinput = 1;
      }
      if (HAL_GPIO_ReadPin(Light_input_GPIO_Port, Light_input_Pin) == Dark && Start_ezinput)
        Dark_time++;
      if (Bright_time > 350 || Dark_time > 350)
      {
        printf("Error in the TimeDetect!\r\n");
        Bright_time = 0;
        Dark_time = 0;
      }
      if (Bright_time / 10 == Dark_time / 10)
      {
        switch (Bright_time / 10)
        {
        case 10:
        {
          printf("Fight!\r\n");
          Bright_time = 0;
          Dark_time = 0;
          Start_ezinput = 0;
          break;
        }
        case 20:
        {
          printf("Retreat!\r\n");
          Bright_time = 0;
          Dark_time = 0;
          Start_ezinput = 0;
          break;
        }
        case 30:
        {
          printf("Come!\r\n");
          Bright_time = 0;
          Dark_time = 0;
          Start_ezinput = 0;
          break;
        }
        }
      }
      break;
    }
    case INPUT:
    {
      if (HAL_GPIO_ReadPin(Light_input_GPIO_Port, Light_input_Pin) == Bright)
      {
        if (!Start_input)
          Start_input = 1;
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
          if (Bright_time == 1)
          {
            Morse[Morse_len++] = '.';
            Bright_time = 0;
            //						printf(".\r\n");
            printf("Morse:%s\r\n", Morse);
          }
          else if (Bright_time == 3)
          {
            Morse[Morse_len++] = '-';
            Bright_time = 0;
            //						printf("-\r\n");
            printf("Morse:%s\r\n", Morse);
          }
          break;
        }
        case 3:
        {
          Morse_to_str(Morse, Str, Morse_len, Str_len++);
          for (int i = 0; i < Str_len; i++)
          {
            printf("%c", Str[i]);
          }
          printf("\r\n");
          Morse_len = 0;
          break;
        }
        case 7:
        {
          Str[Str_len++] = ' ';
          Space_num++;
          break;
        }
        default:
          break;
        }
        if (Dark_time > 7)
        {
          Str[Morse_len - 1] = 0;
          //					T = (Str_len - Space_num) % 7;
          //					Transform_password(Str, Str_len, T - 1);
          printf("EndStr:");
          for (int i = 0; i < Str_len - 1; i++)
          {
            printf("%c", Str[i]);
          }
          printf(".\r\n\r\n");
          memset(Str, 0, Str_len);
          Str_len = 0;
          Space_num = 0;
          Dark_time = 0;
          Start_input = 0;
        }
      }
    }
    default:
      break;
    }
    osDelay(10);
  }
  /* USER CODE END StartTimeDetectTask */
}

/* USER CODE BEGIN Header_StartSignalTask */
/**
* @brief Function implementing the SignalTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSignalTask */
void StartSignalTask(void *argument)
{
  /* USER CODE BEGIN StartSignalTask */
  /* Infinite loop */
  while (1)
  {
    if (PutState == OUTPUT)
    {
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
    }
    else if (PutState == INPUT)
    {
      osSemaphoreAcquire(SignalHandle, osWaitForever);
      Morse_to_signal(Signal_morse, Signal_morse_len);
      memset(Signal_morse, 0, Signal_morse_len);
      Signal_morse_len = 0;
      RxCounter = 0;
    }
  }
  vTaskDelete(NULL);
  /* USER CODE END StartSignalTask */
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
        printf("mode%d.\r\n", LedState + 1);
      }
      else
      {
        LedState = mode1;
        printf("mode1.\r\n");
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

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

