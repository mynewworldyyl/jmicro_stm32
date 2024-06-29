#include "stm32f10x.h"                  // Device header
#include "jm_client.h"
#include "Delay.h"
#include "debug.h"

void CountSensor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
	//GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	//EXTI_InitStructure.EXTI_Line = EXTI_Line1| GPIO_Pin_0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	//NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	//NVIC_Init(&NVIC_InitStructure);
}

void EXTI1_IRQHandler(void)
{
	//SINFO("EXTI1_IRQHandler");
	if (EXTI_GetITStatus(EXTI_Line1) == SET){
		
		static uint32_t lastRunTime = 0;
		if(lastRunTime && ((uint32_t)jm_cli_getSysTime() - lastRunTime < 200)) {
			EXTI_ClearITPendingBit(EXTI_Line1);
			SINFO("EXTI1_IRQHandler1 urt=%u lt=%u\n",jm_cli_getSysTime(),lastRunTime);
			return;
		}
		
		SINFO("EXTI1_IRQHandler2\n");
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0){
			SINFO("post event\n");
			lastRunTime = (uint32_t)jm_cli_getSysTime();
			EXTI_ClearITPendingBit(EXTI_Line1);
			jm_cli_getJmm()->jm_postEvent(TASK_APP_KEY, 1, NULL, JM_EVENT_FLAG_DEFAULT);
			SINFO("post event E\n");
		} else {
			lastRunTime = 0;
			EXTI_ClearITPendingBit(EXTI_Line1);
		}
	}
}

/*
void EXTI0_IRQHandler(void)
{
	//SINFO("EXTI1_IRQHandler");
	if (EXTI_GetITStatus(EXTI_Line0) == SET){
		
		static uint32_t lastRunTime = 0;
		if(lastRunTime && ((uint32_t)jm_cli_getSysTime() - lastRunTime < 200)) {
			EXTI_ClearITPendingBit(EXTI_Line0);
			SINFO("EXTI0_IRQHandler urt=%u lt=%u\n",jm_cli_getSysTime(),lastRunTime);
			return;
		}
		
		SINFO("EXTI0_IRQHandler2\n");
		//如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0){
			SINFO("post0 event\n");
			lastRunTime = (uint32_t)jm_cli_getSysTime();
			//jm_cli_getJmm()->jm_postEvent(TASK_APP_KEY, 1, NULL, JM_EVENT_FLAG_DEFAULT);
		} else {
			lastRunTime = 0;
		}
		
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
*/