#include "stm32f10x.h"                  // Device header
#include "jm_port.h" 
#include "Delay.h"
#include "Timer.h"
#include "jm_net.h"
#include "jm.h"
#include "test.h"
#include "LED.h"
#include "CountSensor.h"
#include "OLED.h"
#include "Key.h"

//STM32入口主函数
int main(void)
{
	
	LED_Init();
	
	//Key_Init();
	CountSensor_Init();
	
	OLED_Init();
	OLED_ShowString(1, 1, "JMicro IOT");
	
	//初始化
	jm_setup();
	
	//0F003 8000F 32343 03930 3142
	//0F003 8000F 32343 03930 31423 33433 36303039343436009446
	uint32_t uid[JM_UNIQUE_ID_LEN/4] = {0};
	if(get_unique_id(uid)) {
		char hexStr[JM_UNIQUE_ID_LEN*2+1];
		jm_toHex((uint8_t*)uid, hexStr, JM_UNIQUE_ID_LEN);
		//OLED_ShowString(3, 1, hexStr);
		OLED_ShowNum(3, 1, uid[0],8);
		SINFO("STM32 UID %s\n",hexStr);
		SINFO("STM32 UID[0] %d\n",uid[0]);
	} else {
		OLED_ShowString(3, 1, "UID Fail");
	}
	
	#if JM_TEST_TCP_ENABLE==1
	jm_test_tcp_init();
	#endif
	
	#if JM_TEST_PRESS_UDP_ENABLE==1
	jm_test_press_udp_init();
	#endif
	
	#if JM_TEST_RPC_ENABLE==1
	jm_test_rpc_init();
	#endif
	
	//UDP报文测试
	#if JM_TEST_UDP_ENABLE==1
	jm_test_udp_init();
	#endif
	
	//RPC压力测试
	#if JM_TEST_PRESS_RPC_ENABLE==1
	jm_test_press_rpc_init();
	#endif
	
	//TCP压力测试
	#if JM_TEST_PRESS_TCP_ENABLE==1
	jm_test_press_tcp_init();
	#endif
	
	//发布/订阅异步消息
	#if JM_TEST_PS_ENABLE==1
	jm_test_ps_init();
	#endif
	
	SINFO("JM STM32 started\n");
	
	while(1) {
		//不断调用此方法
		jm_loop();
	}
	
}
