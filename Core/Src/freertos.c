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
#define Bright GPIO_PIN_RESET
#define Dark GPIO_PIN_SET
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
	OUTPUT = 0,
	EZINPUT = 1,
	INPUT = 2
} PUT_STATE; // ����ģʽ(ͨ�������л�)

typedef enum
{
	mode1 = 0,
	mode2,
	mode3
} LED_STATE;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

PUT_STATE PutState = OUTPUT; // 默认为输出信号模式
LED_STATE LedState = mode1;
uint8_t Key_flag = 0;	   // 按键标志定义
uint32_t Bright_time = 0;  // 代表亮起的单位时间
uint32_t Dark_time = 0;	   // 代表熄灭的单位时间
uint8_t Start_ezinput = 0; // 定义开始接受标志
uint8_t Start_input = 0;   // 定义开始接受标志
uint8_t Morse_len = 0;	   // 定义摩尔斯密码长度
uint8_t Str_len = 0;	   // 定义字符串长度
uint8_t Space_num = 0;	   // 定义字符串中的空格数
uint8_t T = 0;			   // 定义t
uint8_t Morse[50] = {0};   // 定义数组储存摩尔斯密码
uint8_t Str[200] = {0};	   // 定义数组储存字符串

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
	.name = "defaultTask",
	.stack_size = 128 * 4,
	.priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Delay_break(uint16_t ms)
{
	for (uint8_t temp = 0; temp < ms / 10; temp++)
	{
		if (Key_flag)
			break;
		osDelay(10);
	}
}

void Clear_array(uint8_t morse[], uint8_t morse_len)
{
	for (int temp = 0; temp < morse_len; temp++)
		morse[temp] = 0;
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

void Transform(uint8_t morse[], uint8_t str[], uint8_t morse_len, uint8_t str_len)
{
	switch (morse_len)
	{
	case 1:
	{
		if (Judge(morse, ".", 1))
			str[str_len] = 'e';
		if (Judge(morse, "-", 1))
			str[str_len] = 't';
		break;
	}
	case 2:
	{
		if (Judge(morse, ".-", 2))
			str[str_len] = 'a';
		if (Judge(morse, "..", 2))
			str[str_len] = 'i';
		if (Judge(morse, "--", 2))
			str[str_len] = 'm';
		if (Judge(morse, "-.", 2))
			str[str_len] = 'n';
		break;
	}
	case 3:
	{
		if (Judge(morse, "-..", 3))
			str[str_len] = 'd';
		if (Judge(morse, "--.", 3))
			str[str_len] = 'g';
		if (Judge(morse, "-.-", 3))
			str[str_len] = 'k';
		if (Judge(morse, "---", 3))
			str[str_len] = 'o';
		if (Judge(morse, ".-.", 3))
			str[str_len] = 'r';
		if (Judge(morse, "...", 3))
			str[str_len] = 's';
		if (Judge(morse, "..-", 3))
			str[str_len] = 'u';
		if (Judge(morse, ".--", 3))
			str[str_len] = 'w';
		break;
	}
	case 4:
	{
		if (Judge(morse, "-...", 4))
			str[str_len] = 'b';
		if (Judge(morse, "-.-.", 4))
			str[str_len] = 'c';
		if (Judge(morse, "..-.", 4))
			str[str_len] = 'f';
		if (Judge(morse, "....", 4))
			str[str_len] = 'h';
		if (Judge(morse, ".---", 4))
			str[str_len] = 'j';
		if (Judge(morse, ".-..", 4))
			str[str_len] = 'l';
		if (Judge(morse, ".--.", 4))
			str[str_len] = 'p';
		if (Judge(morse, "--.-", 4))
			str[str_len] = 'q';
		if (Judge(morse, "...-", 4))
			str[str_len] = 'v';
		if (Judge(morse, "-..-", 4))
			str[str_len] = 'x';
		if (Judge(morse, "-.--", 4))
			str[str_len] = 'y';
		if (Judge(morse, "--..", 4))
			str[str_len] = 'z';
		break;
	}
	}
}

void LedTask(void *argument)
{
	while (1)
	{
		if (PutState == OUTPUT)
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
}

void KeyScanTask(void *argument)
{
	while (1)
	{
		osDelay(10);
		Key_pressscan(KEY0, &Key_flag);
		if (Key_flag == 1 && PutState == OUTPUT)
		{
			if (LedState < mode3)
				LedState++;
			else
				LedState = mode1;
		}
		if (Key_flag == 3)
		{
			if (PutState < INPUT)
				PutState++;
			else
				PutState = OUTPUT;
		}
	}
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
			if (Bright_time == Dark_time)
			{
				switch (Bright_time)
				{
				case 100:
				{
					printf("Fight!\r\n");
					Bright_time = 0;
					Dark_time = 0;
					Start_ezinput = 0;
					break;
				}
				case 200:
				{
					printf("Retreat!\r\n");
					Bright_time = 0;
					Dark_time = 0;
					Start_ezinput = 0;
					break;
				}
				case 300:
				{
					printf("Come to me!\r\n");
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
				if (Start_input == 0)
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
					}
					else if (Bright_time == 3)
					{
						Morse[Morse_len++] = '-';
						Bright_time = 0;
					}
					break;
				}
				case 3:
				{
					Transform(Morse, Str, Morse_len, Str_len++);
					Clear_array(Morse, Morse_len);
					Morse_len = 0;
				}
				case 7:
				{
					Str[Str_len++] = ' ';
					Space_num++;
				}
				}
				if (Dark_time > 7)
				{
					Str[Morse_len - 1] = 0;
					T = (Str_len - Space_num) % 7;
					Transform_password(Str, Str_len, T);
					for (int i = 0; i < Str_len - 2; i++)
					{
						printf("%c", Str[i]);
					}
					printf(".\r\n");
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
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	xTaskCreate(LedTask, "LedTask", 128, NULL, osPriorityNormal, NULL);
	xTaskCreate(KeyScanTask, "KeyScanTask", 128, NULL, osPriorityNormal, NULL);
	xTaskCreate(TimeDetectTask, "TimeDetectTask", 128, NULL, osPriorityNormal, NULL);
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
	/* Infinite loop */
	for (;;)
	{
		osDelay(1);
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
