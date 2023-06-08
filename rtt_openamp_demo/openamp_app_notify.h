#ifndef __OPENAMP_APP_NOTIFY_H__
#define __OPENAMP_APP_NOTIFY_H__

#include <rtthread.h>
#define M2D_PTY_OPEN      0x00000001
#define M2D_PTY_CLOSE     0x00000002

void openamp_app_notify_cmd(rt_uint32_t cmd);
#endif /* __OPENAMP_APP_NOTIFY_H__ */