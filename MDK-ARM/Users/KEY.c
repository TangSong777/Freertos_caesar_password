#include "KEY.h"

#define KEYn 8

/*BEGIN2
Key_Init(KEY0,KEY0_GPIO_Port,KEY0_Pin.PULL_DOWN);
Key_Init(KEY1,KEY1_GPIO_Port,KEY1_Pin.PULL_DOWN);
Key_Init(KEY2,KEY2_GPIO_Port,KEY2_Pin.PULL_DOWN);
Key_Init(KEY3,KEY3_GPIO_Port,KEY3_Pin.PULL_DOWN);
*/

/*循环
switch(Key0_flag)
{
	case 1:{printf("single click\r\n");Key0_flag=0;break;}
	case 2:{printf("double click\r\n");Key0_flag=0;break;}
	case 3:{printf("long press\r\n");Key0_flag=0;break;}
}
*/

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		if(htim->Instance == TIM10)//0.01s
		{
			Key_scan(KEY0,&Key0_flag);
			Key_scan(KEY1,&Key1_flag);
			Key_scan(KEY2,&Key2_flag);
			Key_scan(KEY3,&Key3_flag);
		}
	}
*/

typedef struct
{
	GPIO_TypeDef *Port; // 端口号
	uint16_t Pin;		// 引脚号
	KEY_DRIVE Level;	// 驱动方式
	KEY_STATE State;	// 按键状态
	KEY_FLAG Flag;		// 有效标志
	uint8_t Time;
	uint8_t Press;
	uint8_t PressTime;
	int Count;
} KEY_TypeDef;

static KEY_TypeDef Keys[KEYn] = {0};

void Key_Init(KEY_INDEX num, GPIO_TypeDef *port, uint16_t pin, KEY_DRIVE level)
{
	Keys[num].Port = port;
	Keys[num].Pin = pin;
	Keys[num].Level = level;
	Keys[num].Flag = 0;
	Keys[num].State = KEY_CHECK;
	Keys[num].Time = 0;
	Keys[num].Press = 0;
	Keys[num].PressTime = 0;
	Keys[num].Count = 0;
}

void Key_normalscan(KEY_INDEX num, uint8_t *Key_flag)
{
	switch (Keys[num].State)
	{
	case KEY_CHECK:
	{
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) == Keys[num].Level)
			Keys[num].State = KEY_COMFIRM;
		break;
	}
	case KEY_COMFIRM:
	{
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) == Keys[num].Level)
		{
			*Key_flag = 1;
			Keys[num].State = KEY_RELEASE;
		}
		else
			Keys[num].State = KEY_CHECK;
		break;
	}
	case KEY_RELEASE:
	{
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) != Keys[num].Level)
			Keys[num].State = KEY_CHECK;
		break;
	}
	default:
		break;
	}
}

void Key_specialscan(KEY_INDEX num, uint8_t *Key_flag)
{
	switch (Keys[num].State)
	{
	case KEY_CHECK:
	{
		if (Keys[num].Time)
			Keys[num].Time--;
		if (Keys[num].Time == 0 && Keys[num].Count == 1)
		{
			*Key_flag = 1;
			Keys[num].Count = 0;
		}
		Keys[num].Press = 0;
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) == Keys[num].Level)
			Keys[num].State = KEY_COMFIRM;
		break;
	}
	case KEY_COMFIRM:
	{
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) == Keys[num].Level)
		{
			if (Keys[num].Time > 0 && Keys[num].Count == 1)
			{
				*Key_flag = 2;
				Keys[num].Count = -1;
			}
			Keys[num].Count++;
			if (Keys[num].Count == 1)
			{
				Keys[num].Time = 30;
				Keys[num].PressTime = 100;
			}
			if (Keys[num].Count == 0)
				Keys[num].Press = 1;
			Keys[num].State = KEY_RELEASE;
		}
		else
			Keys[num].State = KEY_CHECK;
		break;
	}
	case KEY_RELEASE:
	{
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) != Keys[num].Level)
			Keys[num].State = KEY_CHECK;
		else if (Keys[num].PressTime)
			Keys[num].PressTime--;
		if (Keys[num].PressTime == 0 && Keys[num].Press == 0)
		{
			*Key_flag = 3;
			Keys[num].Count = 0;
			Keys[num].Press = 1;
		}
		break;
	}
	default:
		break;
	}
}

void Key_pressscan(KEY_INDEX num, uint8_t *Key_flag)
{
	switch (Keys[num].State)
	{
	case KEY_CHECK:
	{
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) == Keys[num].Level)
			Keys[num].State = KEY_COMFIRM;
		break;
	}
	case KEY_COMFIRM:
	{
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) == Keys[num].Level)
		{
			//*Key_flag = 1;
			Keys[num].PressTime = 100;
			Keys[num].State = KEY_RELEASE;
		}
		else
			Keys[num].State = KEY_CHECK;
		break;
	}
	case KEY_RELEASE:
	{
		if (Keys[num].PressTime)
			Keys[num].PressTime--;
		if (Keys[num].PressTime == 0)
			*Key_flag = 3;
		if (HAL_GPIO_ReadPin(Keys[num].Port, Keys[num].Pin) != Keys[num].Level)
		{
			if (Keys[num].PressTime != 0)
				*Key_flag = 1;
			Keys[num].State = KEY_CHECK;
		}
		break;
	}
	default:
		break;
	}
}