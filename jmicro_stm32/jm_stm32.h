#ifndef JMICRO_JM_DEFAULT_H_
#define JMICRO_JM_DEFAULT_H_

#ifndef JM_STM32
#define JM_STM32 1
#endif

#ifndef SLOG_ENABLE
#define SLOG_ENABLE 0
#endif

//不开启心跳，ESP32心跳由网卡负责
#define JM_HB_ENABLE 0

//ESP32登录由网卡负责
#define JM_LOGIN_ENABLE 0

// 1~20  8266系列
// 21~40 ESP32系列
// 41~50 STM32系列
#ifndef BOARD_TYPE
#define BOARD_TYPE 41 //F10X系列
#endif

#if defined(STM32F10X_CL) || defined(STM32F10X_LD_VL) || defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F10X_HD_VL)
#define JM_UNIQUE_ID_LEN 12 
#else
#define JM_UNIQUE_ID_LEN 16 
#endif

#ifndef JM_SERIAL_ENABLE
#define JM_SERIAL_ENABLE 1
#endif

#if JM_SERIAL_ENABLE==1

#ifndef JM_TCP_ENABLE
#define JM_TCP_ENABLE 0
#endif

#ifndef JM_UDP_ENABLE
#define JM_UDP_ENABLE 1
#endif

#endif //JM_SERIAL_ENABLE==1

#ifndef DEBUG_MEMORY
#define DEBUG_MEMORY 0
#endif

#ifndef JM_RPC_ENABLE
#define JM_RPC_ENABLE 1
#endif

#ifndef JM_PS_ENABLE
#define JM_PS_ENABLE 1
#endif

#ifndef JM_KV_ENABLE
#define JM_KV_ENABLE 0
#endif

#ifndef JM_TIMER_ENABLE
#define JM_TIMER_ENABLE 1
#endif

#ifndef JM_ELIST_ENABLE
#define JM_ELIST_ENABLE 1
#endif

#ifndef JM_EMAP_ENABLE
#define JM_EMAP_ENABLE 1
#endif

#ifndef JM_BUF_ENABLE
#define JM_BUF_ENABLE 1
#endif

#ifndef JM_MSG_ENABLE
#define JM_MSG_ENABLE 1
#endif

#if JM_MSG_ENABLE==1 || JM_EMAP_ENABLE==1 ||JM_ELIST_ENABLE==1
#define JM_MSG_EXTRA_ENABLE 1
#endif

#ifndef JM_STD_TIME_ENABLE
#define JM_STD_TIME_ENABLE 1
#endif

#define JM_CLI_DEBUG_ENABLE 0
#define JM_CLI_ERROR_ENABLE 1

#define JM_BUF_DEBUG_ENABLE 0
#define JM_BUF_ERROR_ENABLE 1

#define JM_MSG_DEBUG_ENABLE 0
#define JM_MSG_ERROR_ENABLE 0

#define JM_MEM_DEBUG_ENABLE 0
#define JM_MEM_ERROR_ENABLE 0

#define JM_STD_DEBUG_ENABLE 0
#define JM_STD_ERROR_ENABLE 0

#define JM_TCP_DEBUG_ENABLE 0
#define JM_TCP_ERROR_ENABLE 0

#define JM_SERIAL_DEBUG_ENABLE 1
#define JM_SERIAL_ERROR_ENABLE 1

#define JM_UDP_DEBUG_ENABLE 1
#define JM_UDP_ERROR_ENABLE 1

#if JM_TCP_DEBUG_ENABLE==1
#define JM_TCP_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_TCP_DEBUG(format, ...)
#endif

#if JM_TCP_ERROR_ENABLE==1
#define JM_TCP_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_TCP_ERROR(format, ...)
#endif

#if JM_UDP_DEBUG_ENABLE==1
#define JM_UDP_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_UDP_DEBUG(format, ...)
#endif

#if JM_UDP_ERROR_ENABLE==1
#define JM_UDP_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_UDP_ERROR(format, ...)
#endif

#if JM_SERIAL_DEBUG_ENABLE==1
#define JM_SERIAL_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_SERIAL_DEBUG(format, ...)
#endif

#if JM_SERIAL_ERROR_ENABLE==1
#define JM_SERIAL_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_SERIAL_ERROR(format, ...)
#endif

#endif //JMICRO_JM_DEFAULT_H_
