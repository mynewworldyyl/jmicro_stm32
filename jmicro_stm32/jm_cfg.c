#include "jm_cfg.h"
#include "debug.h"
#include "jm_mem.h"
#include "jm_stdcimpl.h"
#include "jm_fs.h"

#if ESP8266==1
#include "user_interface.h"
#include "osapi.h"
#endif

#ifdef WIN32
#include "test.h"
#include "jm_test_8266.h"
#endif

#ifdef JM_STM32
#include "stm32_adapter.h"
#endif

#define JM_CFG_DEBUG_ENABLE 1
#define JM_CFG_ERROR_ENABLE 1

#if JM_CFG_DEBUG_ENABLE==1
#define JM_CFG_DEBUG(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_CFG_DEBUG(format, ...)
#endif

#if JM_CFG_ERROR_ENABLE==1
#define JM_CFG_ERROR(format, ...) SINFO(format,## __VA_ARGS__)
#else
#define JM_CFG_ERROR(format, ...)
#endif

static int DEFAULT_LAN_PORT = 9091;

sys_config_t sysCfg;

//static SAVE_FLAG saveFlag;

static jm_hashmap_t *cmdPs;

ICACHE_FLASH_ATTR void jm_cfg_logInfo(){
  
	//JM_SERIAL_DEBUG("slogEnable=%d\n",sysCfg.slogEnable);
	
    JM_CFG_DEBUG("deviceTypeName=%s\n",sysCfg.deviceTypeName);
    JM_CFG_DEBUG("device_id=%s\n",sysCfg.deviceId);
    JM_CFG_DEBUG("inited=%d\n",sysCfg.inited);
    JM_CFG_DEBUG("jlogEnable=%d\n",sysCfg.jlogEnable);
    JM_CFG_DEBUG("slogEnable=%d\n",sysCfg.slogEnable);
    JM_CFG_DEBUG("logEnable=%d\n",sysCfg.logEnable);
    JM_CFG_DEBUG("deviceRole=%d\n",sysCfg.deviceRole);
    JM_CFG_DEBUG("devStatus=%d\n",sysCfg.devStatus);
    JM_CFG_DEBUG("storeGpioStatus=%d\n",sysCfg.storeGpioStatus);
    JM_CFG_DEBUG("key=%s\n",sysCfg.key);
    JM_CFG_DEBUG("invokeCode=%s\n",sysCfg.invokeCode);
    JM_CFG_DEBUG("clientId=%ld\n",sysCfg.clientId);
    JM_CFG_DEBUG("groupId=%d\n",sysCfg.grpId);
	
	JM_CFG_DEBUG("actId=%ld\n",sysCfg.actId);
	//JM_CFG_DEBUG("cfg_holder=%d\n",sysCfg.cfg_holder);
	JM_CFG_DEBUG("jlogHost=%s\n",sysCfg.jlogHost);
    JM_CFG_DEBUG("jlogPort=%u\n",sysCfg.jlogPort);
    JM_CFG_DEBUG("use_udp=%d\n",sysCfg.useUdp);
    JM_CFG_DEBUG("ap_pwd=%s\n",sysCfg.ap_pwd);
    JM_CFG_DEBUG("ap_ssid=%s\n",sysCfg.ap_ssid);
    JM_CFG_DEBUG("devicePort=%d\n",sysCfg.devicePort);
	JM_CFG_DEBUG("jm_host=%s\n",sysCfg.jmHost);
    //JM_CFG_DEBUG("jm_hostNetorder: %s\n",sysCfg.jm_hostNetorder);
    JM_CFG_DEBUG("jm_port=%d\n",sysCfg.jmPort);
    JM_CFG_DEBUG("sta_pwd=%s\n",sysCfg.sta_pwd);
    JM_CFG_DEBUG("sta_ssid=%s\n",sysCfg.sta_ssid);
    JM_CFG_DEBUG("sta_type=%d\n",sysCfg.sta_type);
    JM_CFG_DEBUG("sta_useStore=%d\n",sysCfg.sta_useStore);
	JM_CFG_DEBUG("jmDomain=%s\n",sysCfg.jmDomain);
	
	JM_CFG_DEBUG("devUid=%d\n",sysCfg.devUid);


}

//Win32 platform
ICACHE_FLASH_ATTR  void jm_cfg_save(){
    //JM_CFG_DEBUG("cfg_save win32 do save\n");
    JM_CFG_DEBUG("cfg_save f: %d, size:%u\n",CFG_BLOCK_NO, sizeof(sysCfg));
    //int len = file_write(cfgPath, (void*)&sysCfg, sizeof(sysCfg), 1);
    BOOL rst =  fs_write(CFG_BLOCK_NO, (uint8 *)&sysCfg, sizeof(sysCfg));
    if(!rst) {
        JM_CFG_DEBUG("cfg_save F: %d\n", CFG_BLOCK_NO);
    } 
}

ICACHE_FLASH_ATTR void jm_cfg_load(){

	JM_CFG_DEBUG("cfg_load name:%d size: %u\n",CFG_BLOCK_NO, sizeof(sysCfg));
    BOOL rst = fs_read(CFG_BLOCK_NO, (uint8 *)&sysCfg, sizeof(sysCfg));
    JM_CFG_DEBUG("cfg_load r:%d\n", rst);
    jm_cfg_logInfo();

    //int file_read(char *filePath, void *mem, unsigned int byteSize);
    /*  cfgPath = fp;
    int rst = file_read(cfgPath, (void*)&sysCfg, sizeof(sysCfg));
    JM_CFG_DEBUG("读配置文件结果：%d\n", rst);
    jm_cfg_logInfo(); 
    */
}

/**
 * @param ps
 */
ICACHE_FLASH_ATTR static void _cfg_merge(){

    jm_hashmap_t *ps = cmdPs;

    if(ps == NULL) return;
	
#ifndef JM_STM32
    if(jm_hashmap_exist(ps,"devicePort")) {
        char *str = jm_hashmap_get(ps,"devicePort");
        sysCfg.devicePort = jm_atoi(str);
    }
	
    if(jm_hashmap_exist(ps,"jlogPort")) {
        char *str = jm_hashmap_get(ps,"jlogPort");
        sysCfg.jlogPort = jm_atoi(str);
    }

	 if(jm_hashmap_exist(ps,"jlogHost")) {
        char *str = jm_hashmap_get(ps,"jlogHost");
        os_sprintf(sysCfg.jlogHost, str);
    }
	 
#endif
	
	if(jm_hashmap_exist(ps,"jm_host")) {
        char *jmHost = jm_hashmap_get(ps,"jm_host");
        os_sprintf(sysCfg.jmHost, jmHost);
    }

    if(jm_hashmap_exist(ps,"jm_port")) {
        char *str = jm_hashmap_get(ps,"jm_port");
        sysCfg.jmPort = jm_atoi(str);
    }

   /* if(jm_hashmap_exist(ps,"jlogEnable")) {
        char *str = jm_hashmap_get(ps,"jlogEnable");
        sysCfg.jlogEnable = jm_atoi(str);
    }*/

    sysCfg.jlogEnable = false;

    if(jm_hashmap_exist(ps,"slogEnable")) {
        char *str = jm_hashmap_get(ps,"slogEnable");
        sysCfg.slogEnable = jm_atoi(str);
    }

    if(jm_hashmap_exist(ps,"logEnable")) {
        char *str = jm_hashmap_get(ps,"logEnable");
        sysCfg.logEnable = jm_atoi(str);
    }

    if(jm_hashmap_exist(ps,"deviceRole")) {
        char *str = jm_hashmap_get(ps,"deviceRole");
        sysCfg.deviceRole = jm_atoi(str);
    }

}

ICACHE_FLASH_ATTR void  jm_cfg_reset(){
    JM_CFG_DEBUG("cfg_reset B\n");

    uint32_t curStatus = sysCfg.devStatus;

    uint8_t dn[32];
    if(jm_strlen(sysCfg.deviceTypeName) > 0) {
    	os_sprintf(dn,"%s",sysCfg.deviceTypeName);
    } else {
    	uint32_t time = ((uint32_t)jm_cli_getSysTime()) % 1000;
    	os_sprintf(dn,"ESP8266_J%d_\u914d\u7f51",time);//配网
    }

    jm_cli_getJmm()->jm_memset(&sysCfg, 0, sizeof(sysCfg));

    //sysCfg.gpioStatuEnable = 0;
    // sysCfg.gpioStatus = 0;
    //sysCfg.storeGpioStatus = 1;

    if(curStatus == DEV_STATUS_BUND || curStatus == DEV_STATUS_SYNC_INFO) {
        sysCfg.devStatus = DEV_STATUS_RESET;
    } else if(curStatus == DEV_STATUS_UNBUND || curStatus == DEV_STATUS_INIT) {
        sysCfg.devStatus = DEV_STATUS_INIT;
    } else {
        sysCfg.devStatus = curStatus;
    }
    //sysCfg.sta_type = STATION_MODE;
   // JM_CFG_DEBUG("cfg_reset mode: %d\n", sysCfg.sta_type);

    os_sprintf(sysCfg.deviceTypeName,"%s",dn);//配网
    //JM_CFG_DEBUG("cfg_reset dt: %s\n", sysCfg.deviceTypeName);
   

#ifndef JM_STM32
	 //关生一个随机数作为后缀 12+13
    os_sprintf(sysCfg.ap_ssid,"%s",sysCfg.deviceTypeName);
    //JM_CFG_DEBUG("cfg_reset ap_ssid: %s\n", sysCfg.ap_ssid)
	
#if DEVICE_PORT
    sysCfg.devicePort = DEVICE_PORT;
#else
    sysCfg.devicePort = DEFAULT_LAN_PORT;
#endif
	
	#if ESP8266
    sysCfg.sta_type = STATION_MODE; //STATIONAP_MODE;//初始进入配网模式
	#else
	 sysCfg.sta_type = 1; //STATIONAP_MODE;//初始进入配网模式
	#endif
	
#if JLOG_PORT
    sysCfg.jlogPort = JLOG_PORT;
#else
    sysCfg.jlogPort = 9093;
#endif

#ifdef JLOG_HOST
    os_sprintf(sysCfg.jlogHost, JLOG_HOST);
#else
    os_sprintf(sysCfg.jlogHost, "192.168.3.4");
#endif

#if JM_ENV==1 //生产环境
    os_sprintf(sysCfg.jmDomain, "jmicro.cn");
#else
    os_sprintf(sysCfg.jmDomain, "192.168.3.4");
#endif

	sysCfg.sta_useStore = 1;
    sysCfg.useUdp = true;

#endif // end JM_STM32

#if JM_PORT
    sysCfg.jmPort = JM_PORT;
#else
    sysCfg.jmPort = 9092;
#endif

    //JM_CFG_DEBUG("cfg_reset jmPort: %d\n", sysCfg.jmPort);

#ifdef JM_HOST
    os_sprintf(sysCfg.jmHost, JM_HOST);
#else
    os_sprintf(sysCfg.jmHost, "192.168.3.4");
#endif
    //JM_CFG_DEBUG("cfg_reset jmHost: %s\n", sysCfg.jmHost);
    //JM_CFG_DEBUG("cfg_reset jlogPort: %d\n", sysCfg.jlogPort);
   // JM_CFG_DEBUG("cfg_reset jlogHost: %s\n", sysCfg.jlogHost);

/*
#if JLOG_ENABLE
    sysCfg.jlogEnable = JLOG_ENABLE;
#else
    sysCfg.jlogEnable = false;
#endif
*/
    sysCfg.jlogEnable = false;
    //JM_CFG_DEBUG("cfg_reset jlogEnable: %d\n", sysCfg.jlogEnable);

#if SLOG_ENABLE
    sysCfg.slogEnable = SLOG_ENABLE;
#else
    sysCfg.slogEnable = true;
#endif
    //JM_CFG_DEBUG("cfg_reset slogEnable: %d\n", sysCfg.slogEnable);

#ifdef IS_MASTER
    sysCfg.deviceRole = IS_MASTER;
#else
    sysCfg.deviceRole = 0;
#endif
   // JM_CFG_DEBUG("cfg_reset deviceRole: %d\n", sysCfg.deviceRole);
	
    sysCfg.inited = 1;
    sysCfg.devStatus = DEV_STATUS_RESET;
   

    //JM_CFG_DEBUG("cfg_reset _cfg_merge\n");
    _cfg_merge();

    JM_CFG_DEBUG("cfg_reset jm_cfg_logInfo\n");
    jm_cfg_logInfo();

    //JM_CFG_DEBUG("cfg_reset cfg_save beg\n");
    jm_cfg_save();

    //JM_CFG_DEBUG("cfg_reset finish\n");
}

ICACHE_FLASH_ATTR char* jm_cfg_getCmdPs(char *key){
    if(cmdPs == NULL || cmdPs->size == 0) return NULL;
    return jm_hashmap_get(cmdPs,key);
}

ICACHE_FLASH_ATTR void jm_cfg_enableSlog(){
	sysCfg.slogEnable = true;
	sysCfg.jlogEnable = true;
}

#if defined(STM32F10X_CL) || defined(STM32F10X_LD_VL) || defined(STM32F10X_MD) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F10X_HD_VL)

#include "stm32f10x.h"
 
BOOL get_unique_id(uint32_t *id) {
    id[0] = *(uint32_t *)0x1FFFF7E8;
    id[1] = *(uint32_t *)0x1FFFF7EC;
    id[2] = *(uint32_t *)0x1FFFF7F0;
	return true;
}
 
#elif defined(STM32F40_41xxx) || defined(STM32F427_437xx) || defined(STM32F429_439xx) || defined(STM32F401xx) || defined(STM32F446) || defined(STM32F469_479xx)
 
#include "stm32f4xx.h"
 
BOOL get_unique_id(uint32_t *id) {
    id[0] = *(uint32_t *)0x1FFF7A10;
    id[1] = *(uint32_t *)0x1FFF7A14;
    id[2] = *(uint32_t *)0x1FFF7A18;
    id[3] = *(uint32_t *)0x1FFF7A1C;
	return true;
}
 
#elif defined(STM32H743xx) || defined(STM32H750xx)
 
#include "stm32h743xx.h"
 
BOOL get_unique_id(uint32_t *id) {
    id[0] = *(uint32_t *)0x1FFF7A10;
    id[1] = *(uint32_t *)0x1FFF7A14;
    id[2] = *(uint32_t *)0x1FFF7A18;
    id[3] = *(uint32_t *)0x1FFF7A1C;
	return true;
}
 
#else
#error "Unsupported STM32 microcontroller"
BOOL get_unique_id(uint32_t *id) {
	return false;
}
#endif
 
ICACHE_FLASH_ATTR void jm_init_cfg(jm_mem_op *jmm, jm_hashmap_t *ps){

    JM_CFG_DEBUG("init_cfg init\n");

    //命令行参数
    cmdPs = ps;

    jmm->jm_memset(&sysCfg, 0, sizeof(sysCfg));

     jm_cfg_reset();
	
    _cfg_merge();
	
	jm_cfg_enableSlog();

    //sysCfg.slogEnable = 1;
    //sysCfg.jlogEnable = 0;
    //sysCfg.storeGpioStatus=1;
    //JM_CFG_DEBUG("init_cfg not first running\n");
}
