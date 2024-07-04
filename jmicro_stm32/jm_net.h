#ifndef JM_NET_H_
#define JM_NET_H_

#include "jm_client.h"

#define JM_MAX_SERIAL_BLOCK_SIZE 1024

#define TCP_MAX_SIZE (1024*5)
#define UDP_MAX_SIZE (1024*5)

//#define BUFSIZ 512

//#define READ_BUF_DATA_TIMEOUT 2000 //缓存中数据超时时间
#define UDP_SEND_BUF_SIZE 5 //并发发送包数量

#define UDP_SINGLE_PCK_REPEAT_SIZE 5 //能够缓存多少个防止重复包

#define UDP_SEND_BUF_SIZE 5 //并发发送包数量
#define UDP_PREFIX_BYTE_NUM 3
#define UDP_MAX_PACKAGE_SIZE 128 //发送队列最大数量
#define UDP_READ_BUF_REP_TIMEOUT 2000
#define UDP_READ_BUF_DATA_TIMEOUT (5*UDP_READ_BUF_REP_TIMEOUT)
#define UDP_TOTAL_PACKAGE_SIZE (UDP_PREFIX_BYTE_NUM+UDP_MAX_PACKAGE_SIZE) //发送队列最大数量

#define UDP_FLAG_PROXY_PCK  (1 << 2) //代理转发包

#define UDP_IDX_GRP  0 //数组包组索引
#define UDP_IDX_SEQ  1 //数据包序列索引
#define UDP_IDX_FLAG  2 //标志字节索引
#define UDP_IDX_GRP_VAL  3 //确认包中组值索引
#define UDP_IDX_VAL_NUM  4 //确认包序号个数索引
#define UDP_IDX_DATA_START  5 //确认包中数据索引开始

#define UDP_FLAG_BR  (1 << 0) //是否是广播包
#define UDP_FLAG_NEED_CONFIRM  (1 << 1) //是否需要确认

#define UDP_FLAG_IS(flag, mask) ((flag) & (mask))
#define UDP_FLAG_SET(flag, mask) ((flag) |= (mask))
#define UDP_FLAG_CLEAR(flag, mask) ((flag) &= (~mask))
#define UDP_FLAG(who, flag, mask) ((who)?(UDP_FLAG_SET(flag,mask)):(UDP_FLAG_CLEAR(flag,mask)))

#define SEND_QUEUE_SIZE 20 //发送队列最大数量

#define MS_FOR_SECONDS 1000

//#define KEEPALIVE 512
//#define JM_RECONNECT_TIMEOUT 3000

#define JM_TASK_PRIO              2
#define JM_TASK_QUEUE_SIZE        1
#define JM_SEND_TIMOUT            5

#define JM_SERIALNET_TYPE_UDP 1 //UDP代理
#define JM_SERIALNET_TYPE_TCP 2 //TCP代理
#define JM_SERIALNET_TYPE_SERIAL 3 //串口通信
#define JM_SERIALNET_TYPE_UDP_COM 4 //能用UDP，不处理拆包，组包

#define JM_TASK_APP_PROXY_TCP_CONNECTED (1)  //TCP代理事件字类型 连接成功
#define JM_TASK_APP_PROXY_TCP_DISCONNECTED (2)  //TCP代理事件字类型 连接断开
#define JM_TASK_APP_PROXY_TCP_SEND 3 //TCP发送数据结果
#define JM_TASK_APP_PROXY_TCP_CONN 4 //TCP连接结果
#define JM_TASK_APP_PROXY_TCP_ERR 5 //TCP连接结果

#define JM_TASK_APP_PROXY_WIFI_CFG 6 //Wifi配网
#define JM_TASK_APP_PROXY_WIFI_CONNECTED 7 //Wifi连接成功
#define JM_TASK_APP_PROXY_WIFI_FAIL 8 //Wifi连接失败
#define JM_TASK_APP_PROXY_WIFI_NET_FAIL 9 //Wifi连接,但互联网访问失败
#define JM_TASK_APP_PROXY_INTERNET_ENABLE 10 //查询是否可以访问互联网
#define JM_TASK_APP_PROXY_WIFI_GET_SSID 11 //获取当前连接的Wifi名称
#define JM_TASK_APP_PROXY_WIFI_IS_ENABLE 12 //Wifi是否可用
#define JM_TASK_APP_PROXY_HB 13 //网卡心跳

#define JM_TASK_APP_PROXY_TCP_CLOSE 14 //关闭TCP连接
#define JM_TASK_APP_PROXY_UID 15 //发送或接收单片机唯一ID

//#define JM_TASK_APP_ML 16 //主从事件

#define JM_TASK_APP_PROXY_LOGIN_RESULT 17 //发送或接收单片机唯一ID

#define JM_TASK_APP_PROXY_BR 18 //允许向外广播或响应广播

#define JM_TASK_APP_PROXY_LOGIN 19 //开始登录JM平台

#define JM_TASK_APP_PROXY_SYS_CFG 20 //下发配置信息

#define JM_TASK_APP_PROXY_TRANS_CMD 21 //下发命令到设备

//TCP连接类型
typedef int8_t jm_tcp_socket_t;

typedef struct {
	char *host;
	uint16_t port;
	jm_tcp_socket_t sock;
	sint8_t errCode;
	char *errMsg;
	//uint8_t uuse;
} jm_tcp_conn_t;

typedef struct {
	jm_tcp_socket_t sock;
	uint8_t code;
	char *err;
} jm_tcp_send_result_t;

typedef void (*jm_tcp_onData_fn)(const char* host, int port, jm_buf_t *buf);

typedef void (*jm_udp_onData_fn)(const char* host, int port, jm_buf_t *buf);

void jm_udpclient_init(int8_t uno);

ICACHE_FLASH_ATTR void jm_udp_setDataCb(jm_udp_onData_fn cb);

//UDP发送数据
sint8_t ICACHE_FLASH_ATTR jm_udp_sendBuf(char* host, int port, jm_buf_t *buf);
sint8_t ICACHE_FLASH_ATTR jm_udp_sendArray(char* host, int port, uint8_t *data, uint16_t len);
	
void ICACHE_FLASH_ATTR jm_serial_init();

//接收UDP数据报文
//void jm_upd_onData(const char* host, int port, jm_buf_t *buf);

//接收UDP数据报文
void jm_tcp_onData(const char* host, int port, jm_buf_t *buf);

//接收串口报文
void jm_serial_onData(jm_buf_t *buf);

/**********************************TCP模块开始*********************************************/
//TCP模块初始化
void ICACHE_FLASH_ATTR jm_tcp_init();

//设备接收TCP返回数据的方法
ICACHE_FLASH_ATTR void jm_tcp_setDataCb(jm_tcp_onData_fn cb);

//tcp连接建立，返回值大于0表示成功，其他失败
BOOL ICACHE_FLASH_ATTR jm_tcp_connect(char *host, uint16_t port);

//关闭TCP连接
BOOL ICACHE_FLASH_ATTR jm_tcp_close(jm_tcp_socket_t sock);

//tcp发送数据
sint8_t ICACHE_FLASH_ATTR jm_tcp_sendBuf(jm_tcp_socket_t sock, jm_buf_t *buf);

//tcp发送数据
sint8_t ICACHE_FLASH_ATTR jm_tcp_sendArray(jm_tcp_socket_t sock, uint8_t *data, uint16_t len);

//建立TCP连接,jm_tcp_connect通过此方法实现TCP连接
BOOL ICACHE_FLASH_ATTR jm_serial_tcpconnect(char *host, uint16_t port);

//关闭TCP连接
BOOL ICACHE_FLASH_ATTR jm_serial_tcpclose(jm_tcp_socket_t sock);

//配置Wifi账号密码
void ICACHE_FLASH_ATTR jm_serial_configWifiPwd(char *ssid, char *pwd, jm_cli_rpc_callback_fn cb);

//Wifi是否可用
void ICACHE_FLASH_ATTR jm_serial_isWifiEnable();

//是否可以访问外网
void ICACHE_FLASH_ATTR jm_serial_isInternetEnable();

//网卡可用
ICACHE_FLASH_ATTR void jm_netcard_connected();

//发送唯一码并了得当前网卡的配置信息
uint8_t ICACHE_FLASH_ATTR jm_serial_sendUniqueId();

//串口登录
void ICACHE_FLASH_ATTR jm_serial_login();

/**********************************TCP模块结束********************************************/

//int ICACHE_FLASH_ATTR jm_resovle_domain_ip(char *domain, ip_addr_t *addr, void *dns_cb);

/**
ICACHE_FLASH_ATTR sint8_t net_offerSendQueue(jm_buf_t *buf, char *host, uint16_t port);
ICACHE_FLASH_ATTR jm_tcpclient_send_item* net_popSendQueue();
ICACHE_FLASH_ATTR void net_rebackSendItem(jm_tcpclient_send_item *it);
ICACHE_FLASH_ATTR uint8_t net_queueSize();
*/

#endif
