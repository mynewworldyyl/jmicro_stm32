//
// Created by yeyulei on 2023-7-23.
//

/**
 * 基于Flash的简单文件操作API
 * 数据区的最后一个扇区用于存储文件目录
 * 最后一个扇区号定义为 data_sector_no = cfg_lastUserDataSectorNo(); 参考jm_cfg.c文件
 * 数据存储扇区编号与物理FLASH扇区编号相反，以倒数形式重新编号，以最大限度避开程序存储区
 * 第一个程序存储区编号为 data_sector_no，表示0， 第二个为（data_sector_no-1），第n个表示为(data_sector_no-n)
 *
 * n最大值：
 * 假如flash大小为4M，参考eagle.app.v6.ld文件有如下程序存储区大小定义 len = 0x5C000，开始位置为10000，对应扇区编号为64K/4K=16
 * 也就是从0到15扇区用于存储系统引导程序，从16号扇区开始，用于存储用户程序，5C000/4K=92个扇区，最大扇区编号为 16+92=108。也就是16到108-1编号的
 * 扇区用于存放用户程序，则108及之后的扇区可以用于存放数据，
 *  irom0_0_seg :                         org = 0x40210000, len = 0x5C000
 *
 * 据上，得到data_sector_no-n>=108  => n<=data_sector_no-108  => n<=cfg_lastUserDataSectorNo()-108
 *
 * 此API只能存放有限的数据，能存服务端的数据，不建议存设备上，因为设备Flash非常有限，且不安全
 *
 * 使用第0，1个扇区做安全存储目录数据，两个扇区互为备份，则如一个扇区数据无效，则取另外一个扇区
 *
 */
#include "debug.h"
#include "jm_fs.h"
#include "jm_client.h"
#include "jm_stdcimpl.h"

#if ESP8266==1
#include "user_interface.h"
#include "osapi.h"
#include "spi_flash.h"
#include "jm_ir.h"
#endif

#ifdef WIN32
#include "test.h"
#endif

typedef struct {
    uint8 flag;
    uint8 pad[3];
} Flag;

#define JM_FS_DEBUG_ENABLE 0
#define JM_FS_ERROR_ENABLE 0

#if FS_DEBUG_ENABLE==1
#define JM_FS_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_FS_DEBUG(format, ...)
#endif

#if JM_FS_ERROR_ENABLE==1
#define JM_FS_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_FS_ERROR(format, ...)
#endif

#define PAGE_SIZE 256

#define FILE_NAME_LEN 9

#define FILE_SECTOR_SIZE 4

//两个头部扇区号互为备份
#define FS_HEADER_SECTOR_NO_0 -3 //文件系统头部扇区号0
#define FS_HEADER_SECTOR_NO_1 -2 //文件系统头部扇区号1

//两个目录扇区号互为备份
#define FS_DIR_SECTOR_NO_0 -1 //文件系统目录扇区号0
#define FS_DIR_SECTOR_NO_1 0 //文件系统目录扇区号1

#define SUPPORT_FILE_SIZE 6 //最多支持多少个文件,值必须为大于0的偶数，如2，4，8，10

#define NEED_SECTOR_BIT_NUM (SUPPORT_FILE_SIZE*4) //标识扇区分配标志所需要Bit数,每个文件最多占用4个扇区共16KB

#define NEED_SECTOR_BYTE_NUM (NEED_SECTOR_BIT_NUM/8) //标识扇区分配标志所需要Byte数


ICACHE_FLASH_ATTR static uint32 _fs_totalSectorNum(){
#if ESP8266==1
    enum flash_size_map size_map = system_get_flash_size_map();

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
          return 128;
        case FLASH_SIZE_8M_MAP_512_512:
           return 256;
        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
           return 512;
        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
          return 1024;
        case FLASH_SIZE_64M_MAP_1024_1024:
           return 20450;
        case FLASH_SIZE_128M_MAP_1024_1024:
           return 4096;
        default:
            return 0;
    }
#else
    return 1024;
#endif
}

//-5后，实际上保留了最后5个扇区，从倒数第6个扇区开始，可以用于存储数据
ICACHE_FLASH_ATTR uint32 fs_lastUserDataSectorNo(void){
#if ESP8266==1
    return _fs_totalSectorNum();
#elif WIN32
    #ifndef TOTAL_FLASH_SECTOR_NUM
        return 1024;//默认4M Flash, 1024个扇区
    #else
        return TOTAL_FLASH_SECTOR_NUM;
    #endif
#else
  return 1024-5;
#endif
}

//计算出物理扇区号
ICACHE_FLASH_ATTR static uint16_t _fs_getPhySectorNo(sint16_t logicSectorNo){

	uint16_t totalSectNum = fs_lastUserDataSectorNo()-6;

	if(totalSectNum == 0) {
        JM_FS_ERROR("_fs_getPhySectorNo not support flash op sectorNo=%d\n",logicSectorNo);
		jm_cli_getJmm()->jm_resetSys("_fs_getPhySectorNo not support flash op \n");
		return false;
	}

	//totalSectNum -= 5;//减去预留最后4个系统扇区 + 一个保留扇区
	//物理扇区号=(T+5)-(n+9),其中T=fs_lastUserDataSectorNo()， n为逻辑扇区号
	//==》 物理扇区号=T-n-4

	return totalSectNum - logicSectorNo;//由逻辑扇区号算出物理扇区号
}

ICACHE_FLASH_ATTR static BOOL _fs_check(sint16_t sectorNo, uint32_t dataSize){
	if(sectorNo % 3 != 0) {
		 JM_FS_ERROR("_fs_check invalid sectorNo=%d\n",sectorNo);
		 jm_cli_getJmm()->jm_resetSys("_fs_checkSectorNo invalid sectorNo");
		 return false;
	}

	if(dataSize % 4 != 0) {
		JM_FS_ERROR("_fs_check invalid ds ds=%d\n",dataSize);
		 jm_cli_getJmm()->jm_resetSys("_fs_check invalid data size");
		 return false;
	}

	return true;
}

#if ESP8266 == 1
ICACHE_FLASH_ATTR static BOOL _fs_write(sint16_t sectorNo, uint8 *data, uint32 dataSize) {

	if(!_fs_check(sectorNo, dataSize)) {
		 return false;
	}

	uint16_t phySectorNo = _fs_getPhySectorNo(sectorNo);//计算出物理扇区号

	Flag flag;
	//读标志位
	SpiFlashOpResult rst = spi_flash_read(phySectorNo * SPI_FLASH_SEC_SIZE, &flag, sizeof(flag));

	if(SPI_FLASH_RESULT_OK != rst) {
		//读flash出错
		JM_FS_ERROR("_fs_write read flag error: %u, sectorNo:%d, phyNo: %u, dataSize:%u\n", rst, sectorNo, phySectorNo, dataSize);
		return false;
	}

	flag.flag = flag.flag == 1 ? 2 : 1;

	uint16_t writeDataSector = phySectorNo - flag.flag;

	JM_FS_DEBUG("_fs_write erase data sectorNo: %d, phySectorNo:%u\n",sectorNo, writeDataSector);

#if JM_IR >-1
	IR_pause();
#endif

	spi_flash_erase_sector(writeDataSector); //擦除


	JM_FS_DEBUG("_fs_write w data phySectorNo:%d, ds: %u\n", writeDataSector, dataSize);

	rst = spi_flash_write(writeDataSector*SPI_FLASH_SEC_SIZE, (uint32 *)data, dataSize);

	if(SPI_FLASH_RESULT_OK != rst) {
		//读flash出错
#if JM_IR >-1
		IR_start();
#endif

		JM_FS_DEBUG("_fs_write data error: %u, sectorNo:%d, phyNo: %u, dataSize:%u\n", rst, sectorNo+1, writeDataSector, dataSize);
		return false;
	}

	JM_FS_DEBUG("_fs_write erase flag sectorNo: %d, phySectorNo:%u\n",sectorNo, phySectorNo);
	spi_flash_erase_sector(phySectorNo); //擦除

	JM_FS_DEBUG("_fs_write w flag=%d, sectorNo: %d, phySectorNo:%u\n",flag.flag,sectorNo, phySectorNo);
	rst = spi_flash_write(phySectorNo*SPI_FLASH_SEC_SIZE, (uint32 *)&flag, sizeof(flag));

#if JM_IR >-1
		IR_start();
#endif

	if(SPI_FLASH_RESULT_OK != rst) {
		//读flash出错
		JM_FS_ERROR("_fs_write flag error: %u, sectorNo:%d, phyNo: %u\n", rst, sectorNo, phySectorNo);
		return false;
	}

	//Flag f = {0};
	//读标志位
	//rst = spi_flash_read(phySectorNo*SPI_FLASH_SEC_SIZE, &f, sizeof(f));
	//JM_FS_DEBUG("_fs_write rw phySectorNo:%d, flag=%d addr=%u\n", phySectorNo, f.flag,phySectorNo*SPI_FLASH_SEC_SIZE);

	JM_FS_DEBUG("_fs_write end phySectorNo:%d, ds: %u\n", phySectorNo, dataSize);

	return true;

}

#else 
ICACHE_FLASH_ATTR static BOOL _fs_write(sint16_t sectorNo, uint8 *data, uint32 dataSize) {
	return true;
}
#endif

ICACHE_FLASH_ATTR BOOL fs_write(sint16_t sectorNo, uint8 *data, uint32 dataSize) {
	if(!_fs_check(sectorNo,dataSize)) {
		 return false;
	}
	return _fs_write(sectorNo, data, dataSize);//跨过前四个系统保贸扇区号
}

/**
 * sectorNo FS逻辑扇区号，从0开始编号
 * pageNo 扇区里面的页号，每页256个字节， 一个扇区总共15页
 * data 长度必须是256字节，不能再大了，小心内存溢出
 */
#if ESP8266==1
ICACHE_FLASH_ATTR BOOL fs_read(sint16_t sectorNo, uint8 *data, uint16_t dataSize) {

	if(!_fs_check(sectorNo, dataSize)) {
		 return false;
	}

	uint16_t phySectorNo = _fs_getPhySectorNo(sectorNo);//计算出物理扇区号

	Flag flag = {0};
	//读标志位
	SpiFlashOpResult rst = spi_flash_read(phySectorNo * SPI_FLASH_SEC_SIZE, &flag, sizeof(flag));

	if(SPI_FLASH_RESULT_OK != rst) {
		//读flash出错
		JM_FS_ERROR("_fs_readBytes read flag error: %u, sectorNo:%d, phyNo: %u, dataSize:%u\n", rst, sectorNo, phySectorNo, dataSize);
		return false;
	}

	if(!(flag.flag == 1 || flag.flag == 2)) {
		JM_FS_ERROR("_fs_readBytes flag invalid sectorNo=%u\n",sectorNo);
		return false;
	}

	uint32_t readDataSector = phySectorNo - flag.flag;

	JM_FS_DEBUG("_fs_readBytes sectorNo: %d, phySectorNo:%d, dataSize: %u\n", sectorNo, readDataSector, dataSize);

	rst = spi_flash_read(readDataSector * SPI_FLASH_SEC_SIZE, data, (uint32_t)dataSize);

	if(SPI_FLASH_RESULT_OK == rst) return true;
	else {
		//读flash出错
        JM_FS_ERROR("_fs_readBytes read flash error: %u, logicSectNo:%d, phyNo: %u, dataSize:%u\n", rst, sectorNo, phySectorNo, dataSize);
		return false;
	}
}
#else 
ICACHE_FLASH_ATTR BOOL fs_read(sint16_t sectorNo, uint8 *data, uint16_t dataSize) {
	return true;
}
#endif

#if ESP8266==1
ICACHE_FLASH_ATTR BOOL fs_exist(sint16_t sectorNo){

	if(!_fs_check(sectorNo, 0)) {
		 JM_FS_ERROR("fs_exist c f sectorNo:%d\n", sectorNo);
		 return false;
	}

	uint16_t phySectorNo = _fs_getPhySectorNo(sectorNo);//计算出物理扇区号

	Flag f;
	//读标志位
	SpiFlashOpResult rst = spi_flash_read(phySectorNo*SPI_FLASH_SEC_SIZE, &f, sizeof(f));
	JM_FS_DEBUG("fs_exist phySectorNo:%d, flag=%d\n", phySectorNo, f.flag);

	if(SPI_FLASH_RESULT_OK != rst) {
		//读flash出错
		JM_FS_ERROR("fs_exist read flag error: %u, sectorNo:%d, phyNo: %u\n", rst, sectorNo, phySectorNo);
		return false;
	}

	JM_FS_DEBUG("fs_exist sectorNo:%d phySectorNo:%u, flag:%d addr=%u\n", sectorNo, phySectorNo, f.flag, phySectorNo*SPI_FLASH_SEC_SIZE);
	return f.flag == 1 || f.flag  == 2;
}

#else 
ICACHE_FLASH_ATTR BOOL fs_exist(sint16_t sectorNo){
	
	return true;
}
#endif


#if ESP8266==1
ICACHE_FLASH_ATTR BOOL fs_reset(sint16_t sectorNo){

	if(!_fs_check(sectorNo, 0)) {
		 return false;
	}

	uint16_t phySectorNo = _fs_getPhySectorNo(sectorNo);//计算出物理扇区号

	Flag flag = {0};
	//读标志位
	SpiFlashOpResult rst = spi_flash_read(phySectorNo * SPI_FLASH_SEC_SIZE, &flag, sizeof(flag));

	if(SPI_FLASH_RESULT_OK != rst) {
		//读flash出错
		JM_FS_ERROR("fs_reset read flag error: %u, sectorNo:%d, phyNo: %u\n", rst, sectorNo, phySectorNo);
		return false;
	}

	if(!(flag.flag == 1 || flag.flag == 2)) {
		JM_FS_ERROR("fs_reset flag invalid flag=%d\n",flag.flag);
		return false;
	}

	flag.flag = flag.flag == 1 ? 2 : 1;

	uint16_t writeDataSector = phySectorNo - flag.flag;

	JM_FS_DEBUG("fs_reset sectorNo: %d, phySectorNo:%d\n",sectorNo,phySectorNo);
	spi_flash_erase_sector(writeDataSector); //擦除
	JM_FS_DEBUG("fs_reset spi_flash_erase_sector finish phySectorNo=%d\n",phySectorNo);

	JM_FS_DEBUG("fs_reset dowrite flag phySectorNo=%d\n",phySectorNo);
	rst = spi_flash_write(phySectorNo*SPI_FLASH_SEC_SIZE, (uint32 *)&flag, sizeof(flag));

	if(SPI_FLASH_RESULT_OK != rst) {
		//读flash出错
		JM_FS_ERROR("fs_reset flag error: %u, sectorNo:%d, phyNo: %un", rst, sectorNo, phySectorNo);
		return false;
	}

	JM_FS_DEBUG("fs_reset endphySectorNo=%d\n",phySectorNo);
	return true;
}
#else
ICACHE_FLASH_ATTR BOOL fs_reset(sint16_t sectorNo){
	return true;
}
#endif

ICACHE_FLASH_ATTR void fs_init(){
	//spi_flash_erase_protect_disable();
#ifdef WIN32
	spi_init();
#else
	
#endif
}

