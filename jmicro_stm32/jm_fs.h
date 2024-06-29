#ifndef __JM_FS_H__
#define __JM_FS_H__

#include "c_types.h"

#ifndef TOTAL_FLASH_SECTOR_NUM
#define TOTAL_FLASH_SECTOR_NUM 1024//默认4M Flash, 1024个扇区
#endif

//每个扇区4KB，也就是每个文件最大为4KB大小
//每个文件占3个扇区， 0号扇区标识当前有效扇区号， 1，2号扇区存数据，具体用那个存数据，由0号扇区确定
#define CFG_BLOCK_NO 0 //配置文件开始扇区号
#define CMD_BLOCK_NO 3 //命令文件开始扇区号
#define KEY_BLOCK_NO 6 //GPIO控制配置文件开始扇区号
#define GPIO_BLOCK_NO 9 //GPIO状态存储

#ifdef __cplusplus
extern "C" {
#endif

ICACHE_FLASH_ATTR uint32 fs_lastUserDataSectorNo(void);
ICACHE_FLASH_ATTR BOOL fs_write(sint16_t sectorNo, uint8 *data, uint32 dataSize);
ICACHE_FLASH_ATTR BOOL fs_read(sint16_t sectorNo, uint8 *data, uint16_t dataSize);

/*
ICACHE_FLASH_ATTR BOOL fs_isEmpty(char *fileName);
ICACHE_FLASH_ATTR BOOL fs_delFile(char *fileName);
ICACHE_FLASH_ATTR BOOL fs_newFile(char *fileName);
ICACHE_FLASH_ATTR BOOL fs_existsFile(char *fileName);
ICACHE_FLASH_ATTR BOOL fs_writeFile(char *fileName, uint8_t *data, uint32_t size, BOOL append);
ICACHE_FLASH_ATTR BOOL fs_readFile(char *fileName, uint8_t *data, uint16_t size);
*/

ICACHE_FLASH_ATTR BOOL fs_exist(sint16_t sectorNo);
ICACHE_FLASH_ATTR BOOL fs_reset(sint16_t sectorNo);
ICACHE_FLASH_ATTR void fs_init();



#ifdef __cplusplus
}
#endif

#endif
