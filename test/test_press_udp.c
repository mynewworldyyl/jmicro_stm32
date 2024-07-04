#include "test.h" 

#if JM_TEST_PRESS_UDP_ENABLE==1
#include "stm32f10x.h"                  // Device header
#include "jm.h" 
#include "jm_net.h" 
#include "Serial.h"

static BOOL stop = false;

ICACHE_FLASH_ATTR void _jm_udp_press_testOnData(const char* host, int port, jm_buf_t *buf){
	uint16_t len = jm_buf_readable_len(buf);
	SINFO("cap=%u rpos=%d wpos=%d len=%d\n",buf->capacity, buf->rpos, buf->wpos,len);
	for(int i = 0; i < len; i++) {
		Serial_Printf(JM_UART1, "%c", buf->data[i+buf->rpos]);
	}
}

static ICACHE_FLASH_ATTR void _jm_udp_press_stop(jm_event_t *evt) {
	stop = !stop;
}

static ICACHE_FLASH_ATTR BOOL _jm_udp_serialproxy_press_checker() {
	if(stop) return true;
	SINFO("jm_udp_sendArray B\n");
	//char *p = "GET /index.html HTTP/1.1\r\nAccept: Accept-Language: zh-CN\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2)\r\nAccept-Encoding: gzip,deflate\r\nAccept-Encoding: gzip,deflate \r\n\r\n";
	char *p = "GET /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n /index.html HTTP/1.1\r\nGET /index.html HTTP/1.1\r\n";
	
	jm_udp_sendArray("192.168.3.4",9092, (uint8_t *)p, jm_strlen(p));
	SINFO("test udp send array E\n");
	return true;
}

void jm_test_press_udp_init(){
	SINFO("jm_test_press_udp_init B\n");
	
	jm_cli_getJmm()->jm_regEventListener(JM_TASK_APP_KEY, _jm_udp_press_stop);
	
	jm_cli_registTimerChecker("_pressUdpC",_jm_udp_serialproxy_press_checker,2,5,false);
	
	jm_udp_setDataCb(_jm_udp_press_testOnData);
	SINFO("jm_test_press_udp_init E\n");
}

#endif //JM_TEST_PRESS_UDP_ENABLE==1
