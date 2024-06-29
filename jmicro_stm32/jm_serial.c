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

#if SERIAL_ENABLE==1
extern int8_t jmUartNo;
static int8_t sendChannelNo = 0;

static BOOL wifiEnable = false; 
static BOOL internetEnable = false;

#if SERIAL_DEBUG_ENABLE==1
static void _jm_printBuf(jm_buf_t *buf) {
	uint16_t len = jm_buf_readable_len(buf);
	for(int i = 0; i < len; i++) {
		Serial_Printf(JM_UART1, "%u", buf->data[i]);
		Serial_Printf(JM_UART1, " ");
	}
}
#endif //#if SERIAL_DEBUG_ENABLE==1

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t _jm_serial_sendMsg(jm_buf_t *buf, jm_emap_t *ps) {
	
}

//接收串口过来的数据
ICACHE_FLASH_ATTR void jm_serial_onData(jm_buf_t *buf) {
	
	
	uint16_t subType = 0;
	jm_buf_get_u16(buf, &subType);
	
	JM_SERIAL_DEBUG("jm_serial_onData B len=%u st=%d\n",jm_buf_readable_len(buf),subType);
	
	//TCP连接结果
	char *host = NULL;
	uint16_t port = 0;
	
	jm_tcp_conn_t *conn = NULL;
	
	switch(subType) {
		case TASK_APP_PROXY_TCP_CONNECTED:
		case TASK_APP_PROXY_TCP_DISCONNECTED:
		case TASK_APP_PROXY_TCP_SEND:
		case TASK_APP_PROXY_TCP_ERR:
		{
			int8_t sflag=0;
			host = jm_buf_readString(buf,&sflag);
			jm_buf_get_u16(buf, &port);
			
			int8_t sock = 0;
			jm_buf_get_s8(buf, &sock);
			
			JM_SERIAL_DEBUG("tcp conn result %s:%u sock=%d\n",host?host:"N", port, sock);
			conn = jm_cli_getJmm()->jm_zalloc_fn(sizeof(jm_tcp_conn_t),0);
			if(conn == NULL) {
				JM_SERIAL_ERROR("MO %s:%u sock=%d\n",host?host:"N", port, sock);
				if(host) {
					jm_utils_releaseStr(host,0);
					host = NULL;
				}
				return;
			}
			
			conn->host = host;
			conn->port = port;
			conn->sock = sock;
		}
	}
	
	switch(subType) {
		case TASK_APP_PROXY_HB:
		{
			BOOL we = false; 
			BOOL ie = false;
			
			jm_buf_get_bool(buf, &we);
			jm_buf_get_bool(buf, &ie);
			
			if(wifiEnable != we) {
				//0表示无Wifi连接， 1表示有连接
				wifiEnable = we;
				jm_cli_getJmm()->jm_postEvent(TASK_APP_WIFI, wifiEnable, NULL, JM_EVENT_FLAG_DEFAULT);
			}
			
			if(internetEnable != ie) {
				//2表示可访问外网， 3表示无外网访问
				internetEnable = ie;
				sint8_t st = ie?2:3;
				jm_cli_getJmm()->jm_postEvent(TASK_APP_WIFI, st, NULL, JM_EVENT_FLAG_DEFAULT);
			}
		}
		break;
		
		case TASK_APP_PROXY_TCP_CONNECTED:
		{
			//conn及host内存由接收者释放, 不管创建成功或失败都发送此消息
			jm_cli_getJmm()->jm_postEvent(TASK_APP_SERIAL, subType, conn, JM_EVENT_FLAG_DEFAULT);
		}
		break;
		
		case TASK_APP_PROXY_TCP_DISCONNECTED: {
			jm_cli_getJmm()->jm_postEvent(TASK_APP_SERIAL, subType, conn, JM_EVENT_FLAG_FREE_DATA);
		}
		break;
		
		case TASK_APP_PROXY_WIFI_CONNECTED: {
			
			BOOL we;
			jm_buf_get_bool(buf,&we);
			
			if(wifiEnable != we) {
				wifiEnable = we;
				jm_cli_getJmm()->jm_postEvent(TASK_APP_WIFI, wifiEnable, NULL, JM_EVENT_FLAG_DEFAULT);
			}
			
			sint8_t sflag = 0;
			char *staipaddr = jm_buf_readString(buf, &sflag);
			char *macStaAddr = jm_buf_readString(buf, &sflag);
			
		}
		break;
		
		case TASK_APP_PROXY_TCP_SEND:
		case TASK_APP_PROXY_TCP_ERR:	
		{
			//TCP发送数据结果，resultCode==0表示成功，其他失败，失败时，err描述失败原因
			int8_t errCode = 0;
			jm_buf_get_s8(buf, &errCode);
			conn->errCode = errCode;
			
			JM_SERIAL_DEBUG("tcp send result sock=%d resultCode=%d\n", conn->sock, errCode);
			//jm_tcp_send_result_t *rst = jm_cli_getJmm()->jm_zalloc_fn(sizeof(jm_tcp_send_result_t),0);
		
			if(errCode < 0) {
				int8_t sflag=0;
				char *err = jm_buf_readString(buf,&sflag);
				conn->errMsg = err;
			}
			
			jm_cli_getJmm()->jm_postEvent(TASK_APP_SERIAL, subType, conn, JM_EVENT_FLAG_FREE_DATA);
			break;
		}
		
		default:
			JM_UDP_ERROR("not support subType=%d\n",subType);
	}
	
	JM_SERIAL_DEBUG("jm_serial_onData E\n");
}

BOOL ICACHE_FLASH_ATTR jm_serial_tcpconnect(char *host, uint16_t port){
	JM_SERIAL_DEBUG("tcpconnect B %s:%u\n",host?host:"N", port);
	
	jm_buf_t *hbuf = jm_buf_create(17);
	
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_SERIAL);
	jm_buf_put_u16(hbuf, TASK_APP_PROXY_TCP_CONN);
	jm_buf_writeString(hbuf, host, jm_strlen(host));
	jm_buf_put_u16(hbuf, port);

	Serial_writeLen(jmUartNo, jm_buf_readable_len(hbuf));
	Serial_writeBuf(jmUartNo, hbuf);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("tcpconnect E\n");
	return true;
}

//serial发送数据
uint8_t ICACHE_FLASH_ATTR jm_serial_send(jm_tcp_socket_t sock, jm_buf_t *buf){
	//Send the query to the server
	JM_SERIAL_DEBUG("_jm_serial_sendMsg B\n");
	
	if(jmUartNo <= 0) {
		JM_SERIAL_ERROR("Uart not init\n");
		return NO_DATA_TO_SEND;
	}

	uint16_t totalLen = jm_buf_readable_len(buf);
	if(totalLen <= 0) {
		JM_SERIAL_ERROR("udp lenE:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}
	
	if(totalLen > TCP_MAX_SIZE) {
		JM_SERIAL_ERROR("udp too max:%d\n",totalLen);
		return NO_DATA_TO_SEND;
	}
	
	totalLen += 2;
	
	JM_SERIAL_DEBUG("udp len=%d\n", totalLen);
	
#if TCP_DEBUG_ENABLE==1
	Serial_Printf(JM_UART1, "%u",(totalLen>>8));
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",(totalLen & 0xFF));
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",JM_SERIALNET_TYPE_TCP);
	Serial_Printf(JM_UART1, " ");
	Serial_Printf(JM_UART1, "%u",sock);
	Serial_Printf(JM_UART1, " ");
	_jm_printBuf(buf);
	Serial_Printf(JM_UART1, "\n");
#endif //UDP_DEBUG_ENABLE

	Serial_writeLen(jmUartNo, totalLen);
	Serial_SendByte(jmUartNo, JM_SERIALNET_TYPE_TCP);
	Serial_SendByte(jmUartNo, sock);
	Serial_writeBuf(jmUartNo, buf);
	JM_SERIAL_DEBUG("serial s E=%d\n",JM_SUCCESS);
	return JM_SUCCESS;
}

//配置Wifi账号密码
void ICACHE_FLASH_ATTR jm_serial_configWifiPwd(char *ssid, char *pwd){
	JM_SERIAL_DEBUG("configWifiPwd B ssid=%s pwd=%s\n",ssid?ssid:"", pwd?pwd:"");
	
	jm_buf_t *hbuf = jm_buf_create(17);
	
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_SERIAL);
	jm_buf_put_u16(hbuf, TASK_APP_PROXY_WIFI_CFG);
	jm_buf_writeString(hbuf, ssid, jm_strlen(ssid));
	jm_buf_writeString(hbuf, pwd, jm_strlen(pwd));

	Serial_writeLen(jmUartNo, jm_buf_readable_len(hbuf));
	Serial_writeBuf(jmUartNo, hbuf);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("configWifiPwd E\n");
}

//查看连接的Wifi名称，注意中文可能乱码，所以最好Wifi名称不要带中文
void ICACHE_FLASH_ATTR jm_serial_getWifiSsid(){
	JM_SERIAL_DEBUG("getWifiSsid B\n");
	
	jm_buf_t *hbuf = jm_buf_create(17);
	
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_SERIAL);
	jm_buf_put_u16(hbuf, TASK_APP_PROXY_WIFI_GET_SSID);
	
	Serial_writeLen(jmUartNo, jm_buf_readable_len(hbuf));
	Serial_writeBuf(jmUartNo, hbuf);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("getWifiSsid E\n");
}

BOOL ICACHE_FLASH_ATTR jm_serial_tcpclose(jm_tcp_socket_t sock){
	JM_SERIAL_DEBUG("tcpclose B\n");
	
	jm_buf_t *hbuf = jm_buf_create(17);
	
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_SERIAL);
	jm_buf_put_u16(hbuf, TASK_APP_PROXY_TCP_CLOSE);
	jm_buf_put_s8(hbuf, sock);
	
	Serial_writeLen(jmUartNo, jm_buf_readable_len(hbuf));
	Serial_writeBuf(jmUartNo, hbuf);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("tcpclose E\n");
	
}

//Wifi是否可用
BOOL ICACHE_FLASH_ATTR jm_serial_isWifiEnable(){
	return wifiEnable;
}

static void ICACHE_FLASH_ATTR jm_serial_hostStarted(){
	JM_SERIAL_DEBUG("hostStarted B\n");
	
	jm_buf_t *hbuf = jm_buf_create(17);
	
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_SERIAL);
	jm_buf_put_u16(hbuf, TASK_APP_PROXY_INTERNET_ENABLE);

	Serial_writeLen(jmUartNo, jm_buf_readable_len(hbuf));
	Serial_writeBuf(jmUartNo, hbuf);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("hostStarted E\n");
}


//是否可以访问外网
BOOL ICACHE_FLASH_ATTR jm_serial_isInternetEnable(){
	return internetEnable;
}

void ICACHE_FLASH_ATTR jm_serial_init(){
	JM_SERIAL_DEBUG("serial init B\n");
	jm_serial_hostStarted();
	JM_SERIAL_DEBUG("serial init E\n");
}

#endif //SERIAL_ENABLE

