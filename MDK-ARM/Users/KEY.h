#ifndef __KEY_H_
#define __KEY_H_

#include "stm32f4xx_hal.h"

typedef enum
{
    KEY0 = 0,
    KEY1 = 1,
    KEY2 = 2,
    KEY3 = 3,
    KEY4 = 4,
    KEY5 = 5,
    KEY6 = 6,
    KEY7 = 7,
} KEY_INDEX;

typedef enum
{
    PULL_UP = 0,
    PULL_DOWN
} KEY_DRIVE;

typedef enum
{
    //	KEY_UP = 0,
    //	KEY_DEBOUNCE,
    //	KEY_WAIT_RELEASE
    KEY_CHECK = 0,
    KEY_COMFIRM,
    KEY_RELEASE
} KEY_STATE;

typedef enum
{
    UN = 0,
    EN
} KEY_FLAG;

void Key_Init(KEY_INDEX num, GPIO_TypeDef *port, uint16_t pin, KEY_DRIVE level);
void Key_normalscan(KEY_INDEX num, uint8_t *Key_flag);
void Key_specialscan(KEY_INDEX num, uint8_t *Key_flag);
void Key_pressscan(KEY_INDEX num, uint8_t *Key_flag);

#endif /* __KEY_H_ */