#ifndef __JM_CTRL_H__
#define __JM_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

void jm_ctrl_init();
	
void jm_ctrl_onCmd(jm_buf_t *buf);

#ifdef __cplusplus
}
#endif

#endif
