#ifndef JMICRO_CONFIG_H_
#define JMICRO_CONFIG_H_

#include "c_types.h"
#include "jm_client.h"
#include "jm_mem.h"
#include "jm_msg.h"

typedef struct {
    uint8 flag;
    uint8 pad[3];
} SAVE_FLAG;

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR void jm_cfg_logInfo();
		
ICACHE_FLASH_ATTR void jm_cfg_enableSlog();

ICACHE_FLASH_ATTR void jm_cfg_reset();

ICACHE_FLASH_ATTR uint32_t jm_cfg_lastUserDataSectorNo(void);

ICACHE_FLASH_ATTR void jm_cfg_save();

ICACHE_FLASH_ATTR void jm_cfg_load();

ICACHE_FLASH_ATTR void jm_init_cfg(jm_mem_op *jmm, jm_hashmap_t *ps);

ICACHE_FLASH_ATTR char* jm_cfg_getCmdPs(char *key);
	
#if JM_STM32 
ICACHE_FLASH_ATTR BOOL get_unique_id(uint32_t *id);	
#endif

#ifdef __cplusplus
}
#endif

#endif /* JMICRO_CONFIG_H_ */
