/*
 *
 *  Created on: 2023年4月17日
 *      Author: yeyulei
 */

#include <stddef.h>
#include "jm.h"
#include "Serial.h"
#include "jm_net.h"
#include "jm_ctrl.h"
#include "jm_client.h"

#if JM_SERIAL_ENABLE==1
extern int8_t jmUartNo;

static int8_t sendChannelNo = 0;

static BOOL wifiEnable = false; 
static BOOL internetEnable = false;

static jm_hashmap_t *rpcWaits = NULL;
static sint32_t reqId = 0;

extern sys_config_t sysCfg;

static uint8_t ICACHE_FLASH_ATTR _jm_serial_isInternetEnable_result(void *data, sint32_t code, char *errMsg, void *arg);

static void _jm_printBuf(jm_buf_t *buf) {
	uint16_t len = jm_buf_readable_len(buf);
	JM_SERIAL_DEBUG("S len=%u\n", len);
	for(int i = 0; i < len; i++) {
		Serial_Printf(JM_UART1, "%u", buf->data[i]);
		Serial_Printf(JM_UART1, " ");
	}
	JM_SERIAL_DEBUG("\n");
}

static uint32_t ICACHE_FLASH_ATTR _jm_serial_getDevUid(){
	uint32_t uid[JM_UNIQUE_ID_LEN/4] = {0};
	get_unique_id(uid);
	return uid[0];
}

static void ICACHE_FLASH_ATTR _jm_serial_doSend(jm_buf_t *hbuf){
#if 1
	_jm_printBuf(hbuf);
#endif
	Serial_writeLen(jmUartNo, jm_buf_readable_len(hbuf));
	Serial_writeBuf(jmUartNo, hbuf);
}

static jm_buf_t* ICACHE_FLASH_ATTR _jm_serial_buf(uint16_t subtype, uint16_t msgId, uint16_t size){
	jm_buf_t *hbuf = jm_buf_create(size+5);
	jm_buf_put_u8(hbuf, JM_SERIALNET_TYPE_SERIAL);
	jm_buf_put_u16(hbuf, subtype);
	jm_buf_put_u16(hbuf, msgId);
	return hbuf;
}

ICACHE_FLASH_ATTR static jm_cli_msg_result_t* _c_GetRpcWaitForResponse(jm_hashmap_t *wait_for_resps, sint32_t msgId){
	return jm_hashmap_get(wait_for_resps, (void*)msgId);
}

ICACHE_FLASH_ATTR static jm_cli_msg_result_t* _c_createRpcWaitForResponse(jm_hashmap_t *wait_for_resps, uint32_t msgId){

    JM_SERIAL_DEBUG("cli cresp %u\n", sizeof(struct _c_msg_result));
    jm_cli_msg_result_t *m = jm_utils_mallocWithError(sizeof(struct _c_msg_result),
		PREFIX_TYPE_CLIENT_MSG_RESULT, "_ccrwfr");

	m->callback = NULL;
	m->msg_id = msgId;
	m->cbArg = NULL;

    jm_hashmap_put(wait_for_resps, (void*)msgId, m);

	return m;
}

/**
 */
ICACHE_FLASH_ATTR static  void _c_rebackRpcWaitRorResponse(jm_cli_msg_result_t *m){
    jm_cli_getJmm()->jm_free_fn(m, sizeof(jm_cli_msg_result_t));
}

sint32_t ICACHE_FLASH_ATTR jm_serial_invoke(jm_buf_t *req, uint16_t msgId, jm_cli_rpc_callback_fn callback, void *cbArgs){
	
	if(callback == NULL) {
       _jm_serial_doSend(req);
    } else {
		
		static uint32_t lastCallTime = 0;
		
		if(lastCallTime && jm_cli_getSysTime()-lastCallTime < 20) {
			 JM_CLI_ERROR("ser busy\n");
			return BUSSY;
		}

        jm_cli_msg_result_t *wait = _c_createRpcWaitForResponse(rpcWaits, msgId);
        if(wait == NULL) {
            JM_CLI_ERROR("ser mmo1\n");
			return MEMORY_OUTOF_RANGE;
        }
		
		lastCallTime = jm_cli_getSysTime();
        //wait->msg = msg;
        wait->callback = callback;
        //wait->in_used = true;
        wait->cbArg = cbArgs;
        wait->startTime = lastCallTime;

        _jm_serial_doSend(req);
		return 0;
    }
	return 0;
}


ICACHE_FLASH_ATTR static BOOL _c_checkRpcTimeout(){

	jm_hashmap_t *wait_for_resps = rpcWaits;
    if(wait_for_resps == NULL || wait_for_resps->size == 0) {
        return true;
    }

    //JM_SERIAL_DEBUG("cli chk to s=%d\n",wait_for_resps->size);

    jm_hash_map_iterator_t ite = { wait_for_resps, NULL, -1,wait_for_resps->ver };

    //BOOL to = false;
    void  *key = 0;
    uint32_t  cur = jm_cli_getSysTime();
    while((key = jm_hashmap_iteNext(&ite)) > 0) {
        jm_cli_msg_result_t *wait = jm_hashmap_get(wait_for_resps, key);
        if(wait == NULL) continue;
		
		if(wait->startTime == 0) {
			 jm_hashmap_iteRemove(&ite);
            _c_rebackRpcWaitRorResponse(wait);
			continue;
		}

        if((cur - wait->startTime) > 9000) {
            if(wait->startTime) {
                JM_CLI_DEBUG("cli to msgId: %d, %uMS, to: %u\n", waits->msg_id,
                     (cur - waits->startTime), rpcTimeout);
                //void *resultMap, sint32_t code, char *errMsg, void *arg
                //to = true;
                wait->callback(NULL, 1, "timeout", wait->cbArg);
            }

            jm_hashmap_iteRemove(&ite);
            _c_rebackRpcWaitRorResponse(wait);
        }
		
    }
    return true;
}

ICACHE_FLASH_ATTR jm_cli_send_msg_result_t _jm_serial_sendMsg(jm_buf_t *buf, jm_emap_t *ps) {
	
}

//接收串口过来的数据
ICACHE_FLASH_ATTR void jm_serial_onData(jm_buf_t *buf) {
	
	uint16_t subType = 0;
	jm_buf_get_u16(buf, &subType);
	
	uint16_t reqId = 0;
	jm_buf_get_u16(buf, &reqId);
	
	JM_SERIAL_DEBUG("jm_serial_onData B len=%u st=%d\n",jm_buf_readable_len(buf),subType);
	
	//TCP连接结果
	char *host = NULL;
	uint16_t port = 0;
	
	jm_tcp_conn_t *conn = NULL;
	
	if(reqId > 0) {
		jm_cli_msg_result_t * wait = _c_GetRpcWaitForResponse(rpcWaits, (uint32_t)reqId);
		if(wait != NULL) {
			wait->callback(buf, 0, NULL, wait->cbArg);
			wait->startTime = 0;
			JM_SERIAL_DEBUG("jm_serial_onData E\n");
			return;
		}
	}
	
	switch(subType) {
		case JM_TASK_APP_PROXY_TCP_CONNECTED:
		case JM_TASK_APP_PROXY_TCP_DISCONNECTED:
		case JM_TASK_APP_PROXY_TCP_SEND:
		case JM_TASK_APP_PROXY_TCP_ERR:
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
		case JM_TASK_APP_PROXY_HB:
		{
			_jm_serial_isInternetEnable_result(buf,0,NULL,NULL);
		}
		break;
		
		case JM_TASK_APP_PROXY_TCP_CONNECTED:
		{
			//conn及host内存由接收者释放, 不管创建成功或失败都发送此消息
			jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_SERIAL, subType, conn, JM_EVENT_FLAG_DEFAULT);
		}
		break;
		
		case JM_TASK_APP_PROXY_TCP_DISCONNECTED: {
			jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_SERIAL, subType, conn, JM_EVENT_FLAG_FREE_DATA);
		}
		break;
		
		case JM_TASK_APP_PROXY_WIFI_CONNECTED: {
			
			BOOL we;
			jm_buf_get_bool(buf,&we);
			
			if(wifiEnable != we) {
				wifiEnable = we;
				jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_WIFI, wifiEnable, NULL, JM_EVENT_FLAG_DEFAULT);
			}
			
			sint8_t sflag = 0;
			char *staipaddr = jm_buf_readString(buf, &sflag);
			char *macStaAddr = jm_buf_readString(buf, &sflag);
			
		}
		break;
		
		case JM_TASK_APP_PROXY_TCP_SEND:
		case JM_TASK_APP_PROXY_TCP_ERR:	
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
			
			jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_SERIAL, subType, conn, JM_EVENT_FLAG_FREE_DATA);
			break;
		}
		
		case JM_TASK_APP_PROXY_UID:	
		{
			sint8_t req = 0;
			jm_buf_get_s8(buf, &req);
			if(req!=0) {
				//网卡请求唯一ID
				jm_serial_sendUniqueId();
			}
		}
		break;
		
		case JM_TASK_APP_PROXY_TRANS_CMD:	//从网卡下发的控制命令
		{
			//接收外部命令
			jm_ctrl_onCmd(buf);
		}
		
		default:
			JM_UDP_ERROR("not support subType=%d\n",subType);
	}
	
	JM_SERIAL_DEBUG("jm_serial_onData E\n");
}

BOOL ICACHE_FLASH_ATTR jm_serial_tcpconnect(char *host, uint16_t port){
	JM_SERIAL_DEBUG("tcpconnect B %s:%u\n",host?host:"N", port);
	
	jm_buf_t *hbuf = _jm_serial_buf(JM_TASK_APP_PROXY_TCP_CONN,0, 17);
	
	jm_buf_writeString(hbuf, host, jm_strlen(host));
	jm_buf_put_u16(hbuf, port);

	_jm_serial_doSend(hbuf);
	
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

BOOL ICACHE_FLASH_ATTR jm_serial_tcpclose(jm_tcp_socket_t sock){
	JM_SERIAL_DEBUG("tcpclose B\n");
	
	jm_buf_t *hbuf = _jm_serial_buf(JM_TASK_APP_PROXY_TCP_CLOSE,0, 10);
	
	jm_buf_put_s8(hbuf, sock);
	
	_jm_serial_doSend(hbuf);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("tcpclose E\n");
	
}

//配置Wifi账号密码
void ICACHE_FLASH_ATTR jm_serial_configWifiPwd(char *ssid, char *pwd, jm_cli_rpc_callback_fn cb){
	JM_SERIAL_DEBUG("configWifiPwd B ssid=%s pwd=%s\n",ssid?ssid:"", pwd?pwd:"");
	
	jm_buf_t *hbuf = _jm_serial_buf(JM_TASK_APP_PROXY_WIFI_CFG,0, 17);
	jm_buf_writeString(hbuf, ssid, jm_strlen(ssid));
	jm_buf_writeString(hbuf, pwd, jm_strlen(pwd));

	_jm_serial_doSend(hbuf);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("configWifiPwd E\n");
}

static uint8_t ICACHE_FLASH_ATTR _jm_serial_sendUniqueId_result(void *data, sint32_t code, char *errMsg, void *arg){
	
	JM_SERIAL_DEBUG("sendUniqueId_result B\n");
	
	jm_buf_t *rbuf = (jm_buf_t *)data;
	sint8_t rstCode = 0;
	jm_buf_get_s8(rbuf, &rstCode);

	if(rstCode != 0) {
		JM_SERIAL_ERROR("sendUniqueId Err\n");
		return 1;
	}
	
	jm_buf_get_bytes(rbuf, (uint8_t*)&sysCfg, sizeof(sysCfg));
	
	jm_cfg_enableSlog();
	
	JM_SERIAL_DEBUG("jm_cfg_logInfo\n");
	
	jm_cfg_logInfo();
	
	//提交唯一ID成功
	jm_netcard_connected();
	
	jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_SERIAL, JM_TASK_APP_PROXY_UID, NULL, 0);
	
	jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_SERIAL, JM_TASK_APP_PROXY_SYS_CFG, NULL, 0);
	
	JM_SERIAL_DEBUG("sendUniqueId_result E\n");
	
	return 0;
}

//serial发送设备唯一标识码给网卡,网卡返回配置信息，同时做登录
uint8_t ICACHE_FLASH_ATTR jm_serial_sendUniqueId(){
	JM_SERIAL_DEBUG("send qid B\n");
	
	uint16_t msgId = ++reqId;
	jm_buf_t *hbuf = _jm_serial_buf(JM_TASK_APP_PROXY_UID, msgId, 10);
	jm_buf_put_u32(hbuf, _jm_serial_getDevUid()); //唯一标识码
	jm_buf_put_u16(hbuf, BOARD_TYPE);
	
	char typeName[20] = {0};
	sprintf(typeName, "STM32_J%d", jm_cli_getSysTime()/100);
	
	jm_buf_writeString(hbuf, typeName, jm_strlen(typeName));

	jm_serial_invoke(hbuf, msgId , _jm_serial_sendUniqueId_result, NULL);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("qid E\n");
}

static uint8_t ICACHE_FLASH_ATTR _jm_serial_isInternetEnable_result(void *data, sint32_t code, char *errMsg, void *arg){
	JM_SERIAL_DEBUG("InternetEnable_result B\n");
	jm_buf_t *rbuf = (jm_buf_t *) data;
	
	BOOL we = false;
	BOOL ie = false;
	
	jm_buf_get_bool(rbuf, &we);
	jm_buf_get_bool(rbuf, &ie);
	
	if(wifiEnable != we) {
		//0表示无Wifi连接， 1表示有连接
		wifiEnable = we;
		jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_WIFI, wifiEnable, NULL, JM_EVENT_FLAG_DEFAULT);
	}
	
	if(internetEnable != ie) {
		//2表示可访问外网， 3表示无外网访问
		internetEnable = ie;
		sint8_t st = ie?2:3;
		jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_WIFI, st, NULL, JM_EVENT_FLAG_DEFAULT);
	}
	
	if(wifiEnable && jm_cli_isBind() && !jm_cli_isLogin()) {
		jm_serial_login();
	}
	
	JM_SERIAL_DEBUG("InternetEnable_result E\n");
	return 0;
}

//Wifi是否可用
void ICACHE_FLASH_ATTR jm_serial_isWifiEnable(){
	JM_SERIAL_DEBUG("WifiE B\n");
	
	uint16_t msgId = ++reqId;
	
	jm_buf_t *hbuf = _jm_serial_buf(JM_TASK_APP_PROXY_INTERNET_ENABLE, msgId, 17);

	jm_serial_invoke(hbuf, msgId , _jm_serial_isInternetEnable_result, NULL);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("WifiE E\n");

}

//是否可以访问外网
void ICACHE_FLASH_ATTR jm_serial_isInternetEnable(){
	jm_serial_isWifiEnable();
}

static uint8_t ICACHE_FLASH_ATTR _jm_serial_login_result(void *data, sint32_t code, char *errMsg, void *arg){
	JM_SERIAL_DEBUG("InternetEnable_result B\n");
	jm_buf_t *rbuf = (jm_buf_t *)data;
	
	uint32_t devUid;
	jm_buf_get_u32(rbuf, &devUid);

	if(_jm_serial_getDevUid() != devUid) {
		JM_SERIAL_ERROR("deviUid invalid B\n");
		return 1;
	}
	
	jm_buf_get_s32(rbuf, &sysCfg.actId);
	jm_buf_get_s32(rbuf, &sysCfg.clientId);
	jm_buf_get_s8(rbuf, &sysCfg.grpId);
	
	int8_t sflag=0;
	char *loginKey = jm_buf_readString(rbuf, &sflag);
	if(!loginKey) {
		sint32_t loginCode=0;
		jm_buf_get_s32(rbuf, &loginCode);
		JM_UDP_ERROR("LoginCode=%ld\n", loginCode);
		jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_LOGIN_RESULT, loginCode, NULL, 0);
	} else {
		jm_cli_setLoginKey(loginKey);
		jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_LOGIN_RESULT, 0, NULL, 0);
	}
	
}

//Wifi是否可用
void ICACHE_FLASH_ATTR jm_serial_login(){
	JM_SERIAL_DEBUG("login B\n");
	
	uint16_t msgId = ++reqId;
	
	jm_buf_t *hbuf = _jm_serial_buf(JM_TASK_APP_PROXY_LOGIN, msgId, 17);

	jm_serial_invoke(hbuf, msgId , _jm_serial_login_result, NULL);
	
	jm_buf_release(hbuf);
	
	JM_SERIAL_DEBUG("login E\n");
}


void ICACHE_FLASH_ATTR jm_serial_init(){
	JM_SERIAL_DEBUG("serial init B\n");
	//jm_serial_hostStarted();
	
	rpcWaits = jm_hashmap_create(5, PREFIX_TYPE_SHORTT);
	
	//jm_cli_getJmm()->jm_regEventListener(JM_TASK_APP_LOGIN_RESULT, _jm_serial_login_event_listener);
	
	jm_cli_registTimerChecker("_S_SRPCC",_c_checkRpcTimeout,3,5,false);
	
	JM_SERIAL_DEBUG("serial init E\n");
}

#endif //JM_SERIAL_ENABLE

