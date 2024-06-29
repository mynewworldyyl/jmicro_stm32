#ifndef _JM_TEST_H_
#define _JM_TEST_H_

#include "jm_cons.h"

#define JM_HOST "47.107.141.158"
#define JM_PORT 9092

#define  JM_TEST_RPC_ENABLE 0 //调用外网RPC方法
#define  JM_TEST_UDP_ENABLE 0 //UDP连接，发送和接收数据
#define  JM_TEST_TCP_ENABLE 0 //通过TCP连接，发送和接收数据

//下面是以上三个功能的重复调用，做压力稳定性测试
#define  JM_TEST_PRESS_UDP_ENABLE 0
#define  JM_TEST_PRESS_RPC_ENABLE 0
#define  JM_TEST_PRESS_TCP_ENABLE 0

#define JM_TEST_PS_ENABLE 1 //发布订阅异步消息

#ifdef __cplusplus
extern "C"
{
#endif

 void jm_test_tcp_init();
 void jm_test_rpc_init();
 void jm_test_udp_init();
 void jm_test_ps_init();
	
 void jm_test_press_udp_init();
 void jm_test_press_rpc_init();
 void jm_test_press_tcp_init();

#ifdef __cplusplus
}
#endif

#endif /* _JM_TEST_H_ */