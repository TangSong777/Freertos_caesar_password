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
#include "cmsis_os.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "KEY.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  OUTPUT = 0,
  EZINPUT = 1,
  INPUT = 2
} PUT_STATE; // 长按按键切换状态

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

/* USER CODE BEGIN PV */

/* Definitions for Signal */
osThreadId_t SignalTaskHandle;
const osThreadAttr_t SignalTask_attributes = {
    .name = "SignalTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Definitions for TimeDetect */
osThreadId_t TimeDetectTaskHandle;
const osThreadAttr_t TimeDetectTask_attributes = {
    .name = "TimeDetectTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Definitions for KeyScan */
osThreadId_t KeyScanTaskHandle;
const osThreadAttr_t KeyScanTask_attributes = {
    .name = "KeyScanTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/********************************************************************************/
uint8_t RxBuffer[2048] = {0}; // 串口数据存储BUFF		长度2048
uint8_t RxFlag = 0;           // 串口接收完成标志符
uint16_t RxCounter = 0;       // 串口长度计数
uint8_t RxTemp[1] = {0};      // 串口数据接收暂存BUFF	长度1

extern osSemaphoreId UsartHandle; // 操作系统定义的互斥量
/********************************************************************************/
extern osSemaphoreId SignalHandle;
PUT_STATE PutState = OUTPUT; // 默认为输出信号模式
LED_STATE LedState = mode1;
uint8_t Key_flag = 0;      // 按键标志定义
uint32_t Bright_time = 0;  // 代表亮起的单位时间
uint32_t Dark_time = 0;    // 代表熄灭的单位时间
uint8_t Start_ezinput = 0; // 定义开始接收标志
uint8_t Start_input = 0;   // 定义开始接收标志
uint8_t Start_flash = 0;   // 定义开始转换标志
uint8_t Morse_len = 0;     // 定义摩尔斯密码长度
uint8_t Str_len = 0;       // 定义字符串长度
uint8_t Space_num = 0;     // 定义字符串中的空格数
uint8_t T = 0;             // 定义T
uint8_t Morse[50] = {0};   // 定义数组储存摩尔斯密码
uint8_t Str[200] = {0};    // 定义数组储存字符串
uint8_t Signal_morse[100] = {0};
uint8_t Signal_morse_len = 0;
uint8_t test = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
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
void SignalTask(void *argument)
{
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
}

void TimeDetectTask(void *argument)
{

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
}

void KeyScanTask(void *argument)
{
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
}
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */
  Key_Init(KEY0, KEY_GPIO_Port, KEY_Pin, PULL_UP);

  SignalTaskHandle = osThreadNew(SignalTask, NULL, &SignalTask_attributes);

  TimeDetectTaskHandle = osThreadNew(TimeDetectTask, NULL, &TimeDetectTask_attributes);

  KeyScanTaskHandle = osThreadNew(KeyScanTask, NULL, &KeyScanTask_attributes);

  /* creation of KeyQueue */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) // 锟斤拷锟斤拷2锟斤拷锟斤拷锟斤拷苫氐锟斤拷锟斤拷锟�?
{
  if (huart->Instance == USART2)
  {
    __HAL_TIM_SET_COUNTER(&htim9, 0); // 锟斤拷锟斤拷锟绞憋拷锟�?9锟斤拷锟斤拷值
    if (RxCounter == 0)               // 锟斤拷锟斤拷锟斤拷锟斤拷址锟斤拷锟矫恐★拷锟斤拷菘锟酵凤拷锟斤拷锟斤拷锟斤拷锟绞憋拷锟�?
    {
      __HAL_TIM_CLEAR_FLAG(&htim9, TIM_FLAG_UPDATE); // 锟斤拷锟斤拷卸媳锟街撅拷?
      HAL_TIM_Base_Start_IT(&htim9);                 // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷
    }
    RxBuffer[RxCounter] = RxTemp[0];                    // 锟斤拷锟斤拷锟斤拷锟捷凤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�?
    RxCounter++;                                        // 锟斤拷锟斤拷锟斤拷锟斤拷1
    HAL_UART_Receive_IT(&huart2, (uint8_t *)RxTemp, 1); // 锟斤拷锟斤拷使锟斤拷锟叫讹拷
  }
}
/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM11 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM11)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if (htim->Instance == TIM9)
  {
    RxFlag = 1;                      // 锟斤拷锟秸憋拷志位锟斤拷1
    HAL_TIM_Base_Stop_IT(&htim9);    // 锟截闭讹拷时锟斤拷
    osSemaphoreRelease(UsartHandle); // 锟酵放讹拷值锟脚猴拷锟斤拷锟斤拷锟斤拷锟诫串锟斤拷锟斤拷锟斤拷
  }
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

#ifdef USE_FULL_ASSERT
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
