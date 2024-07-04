#include "test.h" 

#if JM_TEST_PS_ENABLE==1

#include "stm32f10x.h"
#include "jm.h" 
#include "jm_net.h" 
#include "OLED.h" 

#include "LED.h"

static void _jm_rpc_serialproxy_key_listener(jm_event_t *evt) {
	//发布消息
	jm_cli_publishStrItem("/__act/dev/809/testDeivceMsg", -128, "test22222", jm_cli_getUdpExtraByHost("47.107.141.158",9092,true));
}

static void _jm_rpc_feedback(char *msg) {
	//发布消息
	jm_cli_publishStrItem("/__act/dev/809/testDeivceMsg", -128, msg, jm_cli_getUdpExtraByHost("47.107.141.158",9092,true));
}

ICACHE_FLASH_ATTR static uint8_t _ctrl_onPubsubItemTypeListener(jm_pubsub_item_t *item) {
    SINFO("onPubsubItemTypeListener type: %d\n",item->type);
    switch((uint8_t)item->type) {
        case PS_TYPE_CODE(0):
            //0号类型用于测试ESP8266络网连接是否正常
            //test_onTest(item);
		
			SINFO("======%s\n", item->data);
			OLED_Clear();
			OLED_ShowString(2,1,item->data);
		
			if(jm_isDigitStr(item->data)) {
				if(1 == jm_atoi(item->data)) {
					 if(LED1_Turn()) {
						 _jm_rpc_feedback("LED1 On");
					 }else {
						 _jm_rpc_feedback("LED1 Off");
					 }
				}else if(2 == jm_atoi(item->data)) {
					if(LED2_Turn()) {
						 _jm_rpc_feedback("LED2 On");
					 }else {
						 _jm_rpc_feedback("LED2 Off");
					 }
				} else {
					_jm_rpc_feedback("Err Cmd");
				}
			} else {
				char str[50];
				sprintf(str,"From stm32 msg: %s",item->data);
				_jm_rpc_feedback(str);
			}
            break;
        case PS_TYPE_CODE(1):
            //反转15引脚电平信息
            SINFO("ctrl t B\n");
            uint32  gpioNo = atoi((char*)item->data);
#ifdef ESP8266
            jm_key_ledTurn(gpioNo);
#endif

#ifdef WIN32
            SINFO("ctrl on win\n");
#endif
            SINFO("ctrl t E\n");
            break;

#if JM_AT==1
            case PS_TYPE_CODE(2):
		//AT指令
		SINFO("ctrl process AT cmd %s\n",item->data);
		at_fake_uart_rx(item->data,os_strlen(item->data));
		SINFO("ctrl process AT cmd E\n");
		break;
#endif

        case (uint8_t)ITEM_TYPE_CTRL:
            //动态方法调用
            //_ctrl_invokeFunc(item);
            break;
        default:
            SINFO("ctrl not support type: %d\n",item->type);
            break;
    }
	return 0;
}

static ICACHE_FLASH_ATTR void _jm_ctrl_login_event_listener(jm_event_t *e){
	if(!jm_cli_isLogin()) {
        SINFO("NOT Login\n");
        return;
    }
	 //接收远程消息调用（外网），必须在登录成功后才可以订阅成功
    jm_cli_subscribeByType(_ctrl_onPubsubItemTypeListener, 0, true);
}

void jm_test_ps_init(){
	jm_cli_getJmm()->jm_regEventListener(JM_TASK_APP_LOGIN_RESULT, _jm_ctrl_login_event_listener);
	jm_cli_getJmm()->jm_regEventListener(JM_TASK_APP_KEY, _jm_rpc_serialproxy_key_listener);
	//接收本地Wifi环境的消息（内网）
    jm_cli_subscribeByType(_ctrl_onPubsubItemTypeListener, 0, false);
}

#endif //JM_TEST_PS_ENABLE==1
