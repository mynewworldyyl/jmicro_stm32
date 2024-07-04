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

#if JM_UDP_ENABLE==1

int8_t jmUartNo = -1;
static int8_t sendChannelNo = 0;

static jm_udp_onData_fn onUdpData = NULL;

ICACHE_FLASH_ATTR void jm_udp_setDataCb(jm_udp_onData_fn cb){
	onUdpData = cb;
}

#if UDP_DEBUG_ENABLE==1
static void _jm_printBuf(jm_buf_t *buf) {
	uint16_t len = jm_buf_readable_len(buf);
	for(int i = 0; i < len; i++) {
		Serial_Printf(JM_UART1, "%u", buf->data[i]);
		Serial_Printf(JM_UART1, " ");
	}
	Serial_Printf(JM_UART1, "\n");
}
#endif //UDP_DEBUG_ENABLE==1

sint8_t ICACHE_FLASH_ATTR jm_udp_sendArray(char* host, int port, uint8_t *data, uint16_t size){
	jm_buf_t *buf = jm_buf_wrapArrayReadBuf(data, size);
	return jm_udp_sendBuf(host,port, buf);
}

sint8_t ICACHE_FLASH_ATTR jm_udp_sendBuf(char* host, int port, jm_buf_t *buf){
	//Send the query to the server
	JM_TCP_DEBUG("jm_udp_sendBuf B\n");
	
	if(jmUartNo <= 0) {
		JM_TCP_ERROR("Uart not init\n");
		return NO_DATA_TO_SEND;
	}

	uint16_t totalLen = jm_buf_readable_len(buf);
	if(totalLen <= 0) {
		JM_TCP_ERROR("udp lenE:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}
	
	if(totalLen > TCP_MAX_SIZE) {
		JM_TCP_ERROR("udp too max:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}
	
	JM_TCP_DEBUG("udp totalLen=%d\n", totalLen);
	
	jm_buf_t *hbuf = jm_buf_create(17);
	
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_UDP_COM);
	jm_buf_writeString(hbuf, host, jm_strlen(host));
	jm_buf_put_u16(hbuf, port);
	
	totalLen += jm_buf_readable_len(hbuf);
	
	JM_UDP_DEBUG("udp len=%d\n", totalLen);
	
#if 0
	JM_UDP_DEBUG("udp sp\n");
	Serial_Printf(JM_UART1, "%u",(totalLen>>8));
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",(totalLen & 0xFF));
	Serial_Printf(JM_UART1, " ");
	_jm_printBuf(hbuf);
	_jm_printBuf(buf);
	Serial_Printf(JM_UART1, "\n");
#endif //UDP_DEBUG_ENABLE

	Serial_writeLen(jmUartNo, totalLen);
	Serial_writeBuf(jmUartNo, hbuf);
	Serial_writeBuf(jmUartNo, buf);
	
	JM_UDP_DEBUG("udp E=%d\n",JM_SUCCESS);
	return JM_SUCCESS;
	
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t _jm_upd_sendMsg(jm_buf_t *buf, jm_emap_t *ps) {
	//Send the query to the server
	JM_UDP_DEBUG("udp _jm_upd_sendMsg B\n");
	
	if(jmUartNo <= 0) {
		JM_UDP_ERROR("Uart not init\n");
		return NO_DATA_TO_SEND;
	}

	uint16_t totalLen = jm_buf_readable_len(buf);
	if(totalLen <= 0) {
		JM_UDP_ERROR("udp lenE:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}
	
	if(totalLen > UDP_MAX_SIZE) {
		JM_UDP_ERROR("udp too max:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}

	//xx.xx.xx.xx xxxx
	BOOL isUdp = jm_emap_getBool(ps, (void*)EXTRA_KEY_UDP_ACK);
	char *host = jm_emap_getStr(ps, (void*)EXTRA_KEY_UDP_HOST);
	uint16_t port = jm_emap_getInt(ps, (void*)EXTRA_KEY_UDP_PORT);
	
	jm_buf_t *hbuf = jm_buf_create(17);
	
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_UDP);
	jm_buf_writeString(hbuf, host, jm_strlen(host));
	jm_buf_put_u16(hbuf, port);
	
	totalLen += jm_buf_readable_len(hbuf);
	
	JM_UDP_DEBUG("udp len=%d\n", totalLen);
	
#if 0
	JM_UDP_DEBUG("udp sp0\n", totalLen);
	Serial_Printf(JM_UART1, "%u",(totalLen>>8));
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",(totalLen & 0xFF));
	Serial_Printf(JM_UART1, " ");
	_jm_printBuf(hbuf);
	_jm_printBuf(buf);
	Serial_Printf(JM_UART1, "\n");
#endif //UDP_DEBUG_ENABLE

	Serial_writeLen(jmUartNo, totalLen);
	Serial_writeBuf(jmUartNo, hbuf);
	Serial_writeBuf(jmUartNo, buf);
	
	JM_UDP_DEBUG("udp E=%d\n",JM_SUCCESS);
	return JM_SUCCESS;
}

static void _jm_upd_onData(const char* host, int port, jm_buf_t *buf){
	if(onUdpData) {
		onUdpData(host, port , buf);
	}else {
		JM_UDP_ERROR("Got no recv host=%s port=%u len=%u\n",host, port, jm_buf_readable_len(buf));
	}
}

//接收串口过来的数据
ICACHE_FLASH_ATTR void _jm_udpServerRecvData(jm_event_t *evt) {

	if(evt->subType == 0) {
		JM_UDP_ERROR("udp d len=%u\n", evt->subType);
		//Serial_canRecv(true);
		return;
	}
	
	static uint8_t rbuf[JM_MAX_SERIAL_BLOCK_SIZE]={0};
	
	uint16_t len = evt->subType;
	//第一时间取出数据，下面两行代码要必须在第一行，特别是不能放在串口日志输出语句后面
	//uint8_t *p = malloc(len);
	uint8_t bidx = (uint8_t)(evt->data);
	getRXBuffer(rbuf, len, bidx);
	
	JM_UDP_DEBUG("RecvData len=%u bufIdx=%d\n", len, bidx);
#if 0
JM_UDP_DEBUG("udpr p\n");
for(int i=0; i < len; i++) {
	os_printf("%u ",rbuf[i]);
}
os_printf("\n");
#endif //UDP_DEBUG_ENABLE
	
	static uint64_t lastRecvTime = 0;
	static jm_buf_t *rb = NULL;
	
	if(rb && lastRecvTime && (jm_cli_getSysTime() - lastRecvTime) > 500) {
		JM_UDP_ERROR("dis las pck\n");
		jm_buf_release(rb);
		rb = NULL;
	}
	
	jm_buf_t *readBuf = NULL;
	
	if(rb == NULL) {
		//收到第一个串口包
		if(rbuf[0] || rbuf[1]){
			//第一个包，并且需要组包
			//JM_UDP_DEBUG("1 ");
			lastRecvTime = jm_cli_getSysTime();
			uint16 totalSize = rbuf[0] << 8 | rbuf[1];
			if(totalSize == len-2) {//减去包的总长所占两个字节
				//只有一个串口包组成一个物理包
				JM_UDP_DEBUG("one pck total=%u\n",totalSize);
				readBuf = jm_buf_wrapArrayReadBuf(rbuf, len);
				jm_buf_move_forward(readBuf,2);//跳过头部长度的两个字节
			} else {
				JM_UDP_DEBUG("n pck curLen=%u total=%u\n",len, totalSize);
				rb = jm_buf_create(totalSize+2);
				if(!rb) {
					JM_UDP_DEBUG("rbMO=%u\n",totalSize);
					return;
				}
				jm_buf_put_bytes(rb, rbuf, len);
				
				//uint16 ts = rb->data[0] << 8 | rb->data[1];
				//JM_UDP_DEBUG("a ts=%u readable_len=%u\n",ts, jm_buf_readable_len(rb));
				
				jm_buf_move_forward(rb,2);//跳过头部长度的两个字节
				//JM_UDP_DEBUG("a readable_len=%u\n",jm_buf_readable_len(rb));
				return;
			}
		} else {
			JM_UDP_DEBUG("one pck2\n");
			//不需要组包
			readBuf = jm_buf_wrapArrayReadBuf(rbuf, len);
			jm_buf_move_forward(readBuf,2);//跳过头部长度的两个字节
		}
	} else {
		//JM_UDP_DEBUG("3 ");
		lastRecvTime = jm_cli_getSysTime();
		
		uint16 tl = rb->data[0] << 8 | rb->data[1];
		//JM_UDP_DEBUG("b ts=%u\n",tl);
		
		jm_buf_put_chars(rb, (char*)rbuf, len);
		
		//tl = rb->data[0] << 8 | rb->data[1];
		//JM_UDP_DEBUG("b1 ts=%u\n",tl);
		
		uint16_t curLen = jm_buf_readable_len(rb);
		//JM_UDP_DEBUG("b2 curLen=%u\n",curLen);
		
		if(tl == curLen) {
			JM_UDP_DEBUG("Got full pck len=%u\n", tl);
			readBuf = rb;
			rb = NULL;
		} else {
			JM_UDP_DEBUG("Got one bl curLen=%u total=%u\n", curLen, tl);
			return;
		}
	}

	lastRecvTime = 0;
	JM_UDP_DEBUG("udp Recv B len=%u\n", jm_buf_readable_len(readBuf));

	if(readBuf == NULL) {
		//Serial_canRecv(true);
		JM_UDP_ERROR("udp buf MO\n");
		return;
	}
	
	uint8_t type = 0;
	jm_buf_get_u8(readBuf,&type);
	
	JM_UDP_DEBUG("readBuf cap=%u rpos=%d wpos=%d type=%d\n",readBuf->capacity, readBuf->rpos, readBuf->wpos,type);
	
	char *host = NULL;
	uint16_t port = 0;
	if(type == JM_SERIALNET_TYPE_UDP || type == JM_SERIALNET_TYPE_TCP || type == JM_SERIALNET_TYPE_UDP_COM) {
		int8_t sflag=0;
		host = jm_buf_readString(readBuf,&sflag);
		jm_buf_get_u16(readBuf, &port);
		JM_UDP_DEBUG("udp %s:%u\n",host?host:"N", port);
	}

#if JM_RPC_ENABLE==1 || JM_PS_ENABLE==1
	if(type == JM_SERIALNET_TYPE_UDP) {

		jm_msg_t *msg = jm_msg_readMessage(readBuf);

		//Serial_canRecv(true);
	
		if(!msg) {
			JM_UDP_DEBUG("Invalid pck\n");
			jm_buf_release(readBuf);
			return;
		}

		//远程UDP客户端和端口，用于响应信息
		jm_emap_putInt(msg->extraMap, (void*)EXTRA_KEY_UDP_PORT, port,false);
		jm_emap_putStr(msg->extraMap, (void*)EXTRA_KEY_UDP_HOST, host, true,false);
		jm_emap_putBool(msg->extraMap, (void*)EXTRA_KEY_UDP_ACK, true,false);
		jm_emap_putByte(msg->extraMap, (void*)EXTRA_KEY_CHANNEL_NO, sendChannelNo,false);

		jm_msg_setUdp(msg,true);

		sint8_t c = jm_cli_onMessage(msg);
		if(c != JM_SUCCESS) {
			JM_UDP_DEBUG("udp hc: %d\n",c);
		}

		jm_msg_release(msg);
	}else 
	#endif //JM_RPC_ENABLE
	
	if(type == JM_SERIALNET_TYPE_UDP || type == JM_SERIALNET_TYPE_UDP_COM) {
		//if(jm_upd_onData)
		#if JM_UDP_ENABLE==1
			_jm_upd_onData(host, port, readBuf);
		#else
			JM_UDP_ERROR("UDP Dis\n");
		#endif //#if JM_UDP_ENABLE==1
	} else if(type == JM_SERIALNET_TYPE_TCP) {
		#if JM_TCP_ENABLE==1
		//if(jm_tcp_onData)
			jm_tcp_onData(host, port, readBuf);
		#else
		JM_TCP_ERROR("TCP Dis\n");
		//else 
			//JM_UDP_ERROR("TCP no jm_tcp_onData host=%s port=%u\n",host, port);
		#endif //JM_TCP_ENABLE==1
	} else { 
		//type == JM_SERIALNET_TYPE_SERIAL
		#if JM_SERIAL_ENABLE==1
			jm_serial_onData(readBuf);
		#else
			JM_SERIAL_ERROR("SERIAL Dis\n");
		#endif //#if JM_SERIAL_ENABLE==1
	}
	
	jm_buf_release(readBuf);
	readBuf = NULL;
	JM_UDP_DEBUG("udp _jm_udpServerRecvData E\n");
	
	return;
}

void ICACHE_FLASH_ATTR jm_udpclient_init(int8_t uno){
	JM_UDP_DEBUG("udp init B\n");
	if(jmUartNo>=0) {
		JM_UDP_DEBUG("udp have been init\n");
		return;
	}
	
	//初始化UDP的代理串口
	jmUartNo = uno;
	Serial_Init(jmUartNo, NULL, JM_UART_PRO_NETPCK);
	
	//接收串口过来的数据包
	jm_cli_getJmm()->jm_regEventListener(JM_TASK_APP_RX_DATA, _jm_udpServerRecvData);
	
	//向外提供UDP发送接口，将数据通过串口发送出去
	sendChannelNo = jm_cli_registSenderChannel(_jm_upd_sendMsg, JM_UDP_SENDER_NAME);
	
#if JM_SERIAL_ENABLE==1
	jm_serial_init();
#endif
	
#if JM_TCP_ENABLE==1
	jm_tcp_init();
#endif
	
	//UDP连接成功
	jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_UDP, NetConnected, NULL , 0);
	
	JM_UDP_DEBUG("udp init E\n");
}

#endif //JM_UDP_ENABLE==1
