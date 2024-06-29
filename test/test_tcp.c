#include "test.h" 

#if JM_TEST_TCP_ENABLE==1

#include "stm32f10x.h"                  // Device header
#include "jm.h" 
#include "jm_net.h" 

#include "Serial.h" 

static jm_tcp_conn_t *conn = NULL;

static ICACHE_FLASH_ATTR void _jm_serialproxy_listener(jm_event_t *evt) {

	SINFO("serial lis B subType=%u\n",evt->subType);
	if(evt->subType == TASK_APP_PROXY_TCP_CONNECTED) {
		jm_tcp_conn_t *c = (jm_tcp_conn_t *)evt->data;
		SINFO("tcp conn result %s:%u sock=%d\n",c->host?c->host:"N", c->port, c->sock);
		if(c->sock >= 0) {
			SINFO("tcp conn suc\n");
			conn = c;
			char *p = "GET /index.html HTTP/1.1\r\nAccept: Accept-Language: zh-CN\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2)\r\nAccept-Encoding: gzip,deflate\r\nAccept-Encoding: gzip,deflate \r\n\r\n";

			jm_tcp_sendArray(conn->sock, (uint8_t *)p, jm_strlen(p));
		} else {
			SINFO("tcp conn fail\n");
			if(c->host) jm_utils_releaseStr(c->host,0);
			c->host = NULL;
			jm_cli_getJmm()->jm_free_fn(c,sizeof(jm_tcp_conn_t));
			conn = NULL;
		}
	}else if(evt->subType == TASK_APP_PROXY_TCP_DISCONNECTED) {
		//conn = (jm_tcp_conn_t *)evt->data;
		jm_tcp_conn_t *c = (jm_tcp_conn_t *)evt->data;
		SINFO("tcp disconn result %s:%u sock=%d\n",c->host?conn->host:"N", c->port, c->sock);
		
		if(c->host) jm_utils_releaseStr(c->host,0);
		c->host = NULL;
		
		if(conn) {
			if(conn->host) jm_utils_releaseStr(conn->host,0);
			conn->host = NULL;
			jm_cli_getJmm()->jm_free_fn(conn,sizeof(jm_tcp_conn_t));
			conn = NULL;
		}
	}else if(evt->subType == TASK_APP_PROXY_TCP_ERR){
		jm_tcp_conn_t *c = (jm_tcp_conn_t *)evt->data;
		SINFO("tcp send fail %s:%u sock=%d\n",c->host?c->host:"N", c->port, c->sock);
		if(c->host) jm_utils_releaseStr(c->host,0);
		if(c->errMsg) jm_utils_releaseStr(c->errMsg,0);
		c->errMsg = NULL;
		c->host = NULL;
		if(conn) {
			if(conn->host) jm_utils_releaseStr(conn->host,0);
			conn->host = NULL;
			
			if(c->errMsg) jm_utils_releaseStr(c->errMsg,0);
			c->errMsg = NULL;
			
			jm_cli_getJmm()->jm_free_fn(conn,sizeof(jm_tcp_conn_t));
			conn = NULL;
		}
	}
}

static ICACHE_FLASH_ATTR void _jm_tcp_testOnData(const char* host, int port, jm_buf_t *buf){
	uint16_t len = jm_buf_readable_len(buf);
	JM_TCP_DEBUG("cap=%u rpos=%d wpos=%d len=%d\n",buf->capacity, buf->rpos, buf->wpos,len);
	for(int i = 0; i < len; i++) {
		Serial_Printf(JM_UART1, "%c", buf->data[i+buf->rpos]);
	}
}

static ICACHE_FLASH_ATTR void _jm_tcp_serialproxy_key_listener(jm_event_t *evt) {
	if(conn == NULL || conn->sock < 0) {
		/*
		if(!jm_tcp_connect("192.168.3.4",8888)){
			SINFO("TCP Conn Error");
		}*/
		
		if(!jm_tcp_connect("192.168.3.4",9092)){
			SINFO("TCP Conn Error");
		}
	} else {
		/*
		GET /Hello/index.jsp HTTP/1.1     
　　　　Accept: Accept-Language: zh-CN  
　　　　User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E)
　　　　Accept-Encoding: gzip,deflate   
		Host: localhost:8080
　　　　Connection: Keep-Alive  
		*/
		//char *p = "Hello World";
		//char *p = "GET /css/css_jianjie.asp HTTP/1.1\r\nAccept: Accept-Language: zh-CN\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2)\r\nAccept-Encoding: gzip,deflate\r\nConnection: Keep-Alive\r\n\r\n";
		//char *p = "GET /tags/html_ref_httpmethods.asp HTTP/1.1\r\nAccept: Accept-Language: zh-CN\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 8.0;Windows NT 6.1; WOW64; Trident/4.0; SLCC2)\r\nAccept-Encoding: gzip,deflate\r\n\r\n";
		//char *p = "GET /tags/html_ref_httpmethods.asp HTTP/1.1\r\nAccept: Accept-Language: zh-CN\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2)\r\nAccept-Encoding: gzip,deflate\r\nAccept-Encoding: gzip,deflate \r\n\r\n";
		char *p = "GET /index.html HTTP/1.1\r\nAccept: Accept-Language: zh-CN\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2)\r\nAccept-Encoding: gzip,deflate\r\nAccept-Encoding: gzip,deflate \r\n\r\n";

		jm_tcp_sendArray(conn->sock, (uint8_t *)p, jm_strlen(p));
	}
}

void jm_test_tcp_init(){
	
	//TCP连接相关事件
	jm_cli_getJmm()->jm_regEventListener(TASK_APP_SERIAL, _jm_serialproxy_listener);
	
	//按键事件
	jm_cli_getJmm()->jm_regEventListener(TASK_APP_KEY, _jm_tcp_serialproxy_key_listener);
	
	//接收TCP返回数据
	jm_tcp_setDataCb(_jm_tcp_testOnData);
	
	/*
	if(!jm_tcp_connect("39.156.66.10",80)){
		SINFO("TCP Conn Error");
	}
	*/
}

#endif //JM_TEST_TCP_ENABLE==1