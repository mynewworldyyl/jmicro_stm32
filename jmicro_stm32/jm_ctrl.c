//
// Created by yeyulei on 2023-7-23.
//

#include "jm.h"

void jm_ctrl_onCmd(jm_buf_t *buf) {
	int8_t f = false;
	char *cmdStr = jm_buf_readString(buf,&f);
	SINFO("Got cmdStr=%s",cmdStr);
}

void jm_ctrl_init() {
	
}
	


