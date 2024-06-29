#include "stm32f10x.h"                  // Device heade
#include "LED.h"

#define LED1 GPIO_Pin_13
#define LED2 GPIO_Pin_14

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = LED1 | LED2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, LED1 | LED2);
}

void LED1_ON(void)
{
	GPIO_ResetBits(GPIOB, LED1);
}

void LED1_OFF(void)
{
	GPIO_SetBits(GPIOB, LED1);
}

BOOL LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, LED1) == 0)
	{
		GPIO_SetBits(GPIOB, LED1);
		return false;
	}
	else
	{
		GPIO_ResetBits(GPIOB, LED1);
		return true;
	}
}

void LED2_ON(void)
{
	GPIO_ResetBits(GPIOB, LED2);
}

void LED2_OFF(void)
{
	GPIO_SetBits(GPIOB, LED2);
}

BOOL LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, LED2) == 0)
	{
		GPIO_SetBits(GPIOB, LED2);
		return false;
	}
	else
	{
		GPIO_ResetBits(GPIOB, LED2);
		return true;
	}
}
