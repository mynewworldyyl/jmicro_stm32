#ifndef JMICRO_stm32_adapter_JM_test_H_
#define JMICRO_stm32_adapter_JM_test_H_
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "time.h"
#include "c_types.h"

#ifndef OK
//#define OK 1
#endif

#ifndef ICACHE_FLASH_ATTR
#define ICACHE_FLASH_ATTR
#endif

#ifndef sint64_t
#define sint64_t int64_t
#endif

#ifndef BOOL
#define BOOL bool
#endif

#ifndef NULL
#define NULL (void *)0
#endif

#define NULL_MODE       0x00
#define STATION_MODE    0x01
#define SOFTAP_MODE     0x02
#define STATIONAP_MODE  0x03

#define IPADDR_NONE         ((uint32_t)0xffffffffUL)

#define GPIO_ID_PIN0                                     0
#define GPIO_ID_PIN(n)                                   (GPIO_ID_PIN0+(n))

#define os_zalloc malloc
#define os_free free
#define os_realloc realloc

#define os_memcpy memcpy
#define os_memset memset
#define os_strcpy strcpy
#define os_strncmp strncmp
#define os_strcmp strcmp
#define os_strlen strlen
#define os_strncpy strncpy
#define os_printf printf
#define os_sprintf sprintf

#define ICACHE_FLASH_ATTR

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif //JMICRO_stm32_adapter_JM_test_H_



