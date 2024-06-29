#include "test.h" 

#if JM_TEST_PRESS_RPC_ENABLE==1

#include "stm32f10x.h"                  // Device header
#include "jm.h" 
#include "jm_net.h" 

static BOOL stop = false;

static void _c_commonRpcResult(void *r, sint32_t code, char *errMsg, void *arg){

	SINFO("stt cbcode:%u\n",(uint8_t)arg);

	if(code != 0) {
		SINFO("stt code=%ld, msg=%s\n", code, errMsg?errMsg:"NULL");
		goto end;
	}

	uint8_t cbcode =(uint8_t)arg;

	switch(cbcode) {
		case 1:
		{
			jm_emap_t *ps = (jm_emap_t *)r;
			uint32_t val = jm_emap_getInt(ps,"data");
			SINFO("stt t:%u\n", val);
			break;
		}
		default:
			SINFO("stt err cbcode: %d\n",cbcode);
			break;
	}

end:
	//jm_emap_release(r);
	SINFO("stt E\n");

}

static BOOL _jm_rpc_serialproxy_key_listener() {
	sint64_t msgId = jm_cli_invokeRpc(-2072516138, NULL, (jm_cli_rpc_callback_fn)_c_commonRpcResult, (void*)1);
	if(msgId < 0 && msgId != JM_SUCCESS) {
		JM_CLI_ERROR("c up time E");
	}
	return true;
}

static ICACHE_FLASH_ATTR void _jm_udp_press_stop(jm_event_t *evt) {
	stop = !stop;
}

void jm_test_press_rpc_init(){
	SINFO("jm_test_press_udp_init B\n");
	
	jm_cli_getJmm()->jm_regEventListener(TASK_APP_KEY, _jm_udp_press_stop);
	
	jm_cli_registTimerChecker("_pressUdpC",_jm_rpc_serialproxy_key_listener,2,5,false);
	
	SINFO("jm_test_press_udp_init E\n");
}

#endif //JM_TEST_PRESS_RPC_ENABLE==1
