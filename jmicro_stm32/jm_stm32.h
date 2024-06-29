#ifndef JMICRO_JM_DEFAULT_H_
#define JMICRO_JM_DEFAULT_H_

#ifndef JM_STM32
#define JM_STM32 1
#endif

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

#define CLI_DEBUG_ENABLE 0
#define CLI_ERROR_ENABLE 1

#define BUF_DEBUG_ENABLE 0
#define BUF_ERROR_ENABLE 0

#define MSG_DEBUG_ENABLE 0
#define MSG_ERROR_ENABLE 0

#define MEM_DEBUG_ENABLE 0
#define MEM_ERROR_ENABLE 0

#define STD_DEBUG_ENABLE 0
#define STD_ERROR_ENABLE 0

#define TCP_DEBUG_ENABLE 0
#define TCP_ERROR_ENABLE 0

#define SERIAL_DEBUG_ENABLE 0
#define SERIAL_ERROR_ENABLE 1

#define UDP_DEBUG_ENABLE 0
#define UDP_ERROR_ENABLE 1

#if TCP_DEBUG_ENABLE==1
#define JM_TCP_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_TCP_DEBUG(format, ...)
#endif

#if TCP_ERROR_ENABLE==1
#define JM_TCP_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_TCP_ERROR(format, ...)
#endif

#if UDP_DEBUG_ENABLE==1
#define JM_UDP_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_UDP_DEBUG(format, ...)
#endif

#if UDP_ERROR_ENABLE==1
#define JM_UDP_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_UDP_ERROR(format, ...)
#endif

#if SERIAL_DEBUG_ENABLE==1
#define JM_SERIAL_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_SERIAL_DEBUG(format, ...)
#endif

#if SERIAL_ERROR_ENABLE==1
#define JM_SERIAL_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_SERIAL_ERROR(format, ...)
#endif

#endif //JMICRO_JM_DEFAULT_H_
