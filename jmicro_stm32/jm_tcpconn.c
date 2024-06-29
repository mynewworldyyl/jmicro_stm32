/*
 * jm_client_winsocket.c
 *
 *  Created on: 2023年4月17日
 *      Author: yeyulei
 */

#include <stddef.h>
#include "jm.h"
#include "Serial.h"
#include "jm_net.h"

#if TCP_ENABLE==1

extern int8_t jmUartNo;
static int8_t sendChannelNo = 0;

static jm_tcp_onData_fn onTcpData = NULL;

ICACHE_FLASH_ATTR void jm_tcp_setDataCb(jm_tcp_onData_fn cb){
	onTcpData = cb;
}

#if TCP_DEBUG_ENABLE==1
static void _jm_printBuf(jm_buf_t *buf) {
	uint16_t len = jm_buf_readable_len(buf);
	JM_TCP_DEBUG("cap=%u rpos=%d wpos=%d len=%d\n",buf->capacity, buf->rpos, buf->wpos,len);
	for(int i = 0; i < len; i++) {
		Serial_Printf(JM_UART1, "%d", buf->data[i+buf->rpos]);
		Serial_Printf(JM_UART1, " ");
	}
	Serial_Printf(JM_UART1, "\n");
}
#endif // TCP_DEBUG_ENABLE==1

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t _jm_tcp_sendMsg(jm_buf_t *buf, jm_emap_t *ps) {
	
}

//接收串口过来的数据
ICACHE_FLASH_ATTR void jm_tcp_onData(const char* host, int port, jm_buf_t *buf) {
	JM_TCP_DEBUG("jm_tcp_onData Recv B %s:%u len=%u\n",host?host:"N", port, jm_buf_readable_len(buf));
	
	if(onTcpData) {
		onTcpData(host, port,buf);
	} else {
		JM_TCP_ERROR("jm_tcp_onData_fn not set\n");
		uint16_t len = jm_buf_readable_len(buf);
		JM_TCP_DEBUG("cap=%u rpos=%d wpos=%d len=%d\n",buf->capacity, buf->rpos, buf->wpos,len);
		for(int i = 0; i < len; i++) {
			Serial_Printf(JM_UART1, "%c", buf->data[i+buf->rpos]);
		}
	}

	JM_TCP_DEBUG("jm_tcp_onData E\n");
}

BOOL ICACHE_FLASH_ATTR jm_tcp_connect(char *host, uint16_t port){
	JM_TCP_DEBUG("jm_tcp_connect B %s:%u\n",host?host:"N", port);
	return jm_serial_tcpconnect(host,port);
}

BOOL ICACHE_FLASH_ATTR jm_tcp_close(jm_tcp_socket_t sock){
	return jm_serial_tcpclose(sock);
}

//tcp发送数据
sint8_t ICACHE_FLASH_ATTR jm_tcp_sendBuf(jm_tcp_socket_t sock, jm_buf_t *buf){
	//Send the query to the server
	JM_TCP_DEBUG("_jm_tcp_sendMsg B\n");
	
	if(jmUartNo <= 0) {
		JM_TCP_ERROR("Uart not init\n");
		return NO_DATA_TO_SEND;
	}

	uint16_t totalLen = jm_buf_readable_len(buf);
	if(totalLen <= 0) {
		JM_TCP_ERROR("tcp lenE:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}
	
	totalLen += 2;
	
	if(totalLen > TCP_MAX_SIZE) {
		JM_TCP_ERROR("tcp too max:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}
	
	JM_TCP_DEBUG("tcp totalLen=%d\n", totalLen);
	
#if TCP_DEBUG_ENABLE==1
	Serial_Printf(JM_UART1, "%u",(totalLen>>8));
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",(totalLen & 0xFF));
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",JM_SERIALNET_TYPE_TCP);
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",sock);
	Serial_Printf(JM_UART1, " ");
	//_jm_printBuf(buf);
	uint16_t len = jm_buf_readable_len(buf);
	for(int i = 0; i < len; i++) {
		Serial_Printf(JM_UART1, "%d", buf->data[i+buf->rpos]);
		Serial_Printf(JM_UART1, " ");
	}
	Serial_Printf(JM_UART1, "\n");
#endif //TCP_DEBUG_ENABLE

	Serial_writeLen(jmUartNo, totalLen);
	Serial_SendByte(jmUartNo, JM_SERIALNET_TYPE_TCP);
	Serial_SendByte(jmUartNo, sock);
	Serial_writeBuf(jmUartNo, buf);

	JM_TCP_DEBUG("tcp s E=%d\n",JM_SUCCESS);
	return JM_SUCCESS;
}

sint8_t ICACHE_FLASH_ATTR jm_tcp_sendArray(jm_tcp_socket_t sock, uint8_t *data, uint16_t len){
	jm_buf_t *buf = jm_buf_wrapArrayReadBuf(data, len);
	uint8_t rst = jm_tcp_sendBuf(sock, buf);
	jm_buf_release(buf);
	return rst;
}

void ICACHE_FLASH_ATTR jm_tcp_init(){
	JM_TCP_DEBUG("tcp init B\n");
	
	JM_TCP_DEBUG("tcp init E\n");
}

#endif //TCP_ENABLE==1
