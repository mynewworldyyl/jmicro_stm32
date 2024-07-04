#include "stm32f10x.h"                  // Device header
#include "Delay.h"

#include "Serial.h"
#include <string.h>
#include "Timer.h"
#include "jm.h" 
#include "jm_net.h" 
#include "jm_port.h" 

extern void jm_runner_init();

ICACHE_FLASH_ATTR void jm_runner_setEventParam(jm_mem_op *jmm);

static BOOL netCardConnected = false;

static void _jm_onUartData(uint8_t uartNo, uint8_t byte) {
	JM_MAIN_DEBUG("rd uartNo=%d data=%d\n",uartNo, byte);
	/*
	if(l == 0) {
		return;
	}
	
	uint8_t *data = jm_cli_getJmm()->jm_zalloc_fn(l, 0);
	if(data==NULL) {
		JM_MAIN_DEBUG("on uData MO size=%d\n",l);
		return;
	}
	
	jm_cli_getJmm()->jm_memcpy(data, d, l);
	
	user_postEvent(JM_TASK_APP_RX_DATA, 1, data, JM_EVENT_FLAG_FREE_DATA);
	*/
}

ICACHE_FLASH_ATTR static void _error_resetSystem(char *cause) {
	 JM_MAIN_ERROR("_error_resettSystem\n");
	 JM_MAIN_ERROR("cause=%s\n",cause);
	 //system_restart();
	 jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_RESTART_SYSTEM, 1, NULL, JM_EVENT_FLAG_DEFAULT);
}

ICACHE_FLASH_ATTR static void _jm_delay(uint32_t ms) {
	Delay_ms(ms);
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIMER_NO, TIM_IT_Update) == SET)
	{
		jm_ElseMs(1);
		TIM_ClearITPendingBit(TIMER_NO, TIM_IT_Update);
	}
}

ICACHE_FLASH_ATTR void jm_loop() {
	if(!netCardConnected) {
		//SINFO("UniqueId0\n");
		static uint32_t lastCallTime = 0;
		if(jm_cli_getSysTime()-lastCallTime > 1000) {
			netCardConnected = true;
			SINFO("UniqueId1\n");
			jm_serial_sendUniqueId();
			lastCallTime = jm_cli_getSysTime();
		}	
	}
	jm_runEvent();
	IWDG_ReloadCounter();
}

ICACHE_FLASH_ATTR void jm_netcard_connected(){
	netCardConnected = true;
}

ICACHE_FLASH_ATTR void jm_setup(){
	
	//日志输出
	Serial_Init(JM_UART1, _jm_onUartData, JM_UART_PRO_NOMAL);
	
	jm_cfg_enableSlog();
	
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST)==SET) {
		JM_MAIN_ERROR("Reset by IWDG\n");
		RCC_ClearFlag();
	}
	
	Delay_ms(1);
	
	Timer_Init();
	
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_32);
	IWDG_SetReload(2499);
	IWDG_ReloadCounter();
	
	IWDG_Enable();

	jm_mem_op jmm;
	jmm.jm_free_fn = mem_free;
	jmm.jm_realloc_fn = realloc;
	jmm.jm_zalloc_fn = mem_zalloc;
	jmm.jm_memcpy = memcpy;
	jmm.jm_memset = memset;
	jmm.jm_resetSys = _error_resetSystem;
	jmm.jm_delay = _jm_delay;

	//JM_MAIN_DEBUG("POINT LEN=%u\n",sizeof(jm_mem_op*));
	
	//接下来这5个方法调用顺序不能调换
	jm_runner_setEventParam(&jmm);

	jm_cli_setMemOps(jmm);
	
	jm_mem_init();
	
	jm_runner_init();
	//以上这5个方法调用顺序不能调换
	
	//加载配置信息
	JM_MAIN_DEBUG("start_jm config\n");
	jm_init_cfg(&jmm, NULL);
	JM_MAIN_DEBUG("start_jm load cofig end\n");

	jm_cli_init();
	
//udp代理
#if JM_UDP_ENABLE==1
	jm_udpclient_init(JM_UART2);
#endif

	JM_MAIN_DEBUG("start_jm end\n");

}
