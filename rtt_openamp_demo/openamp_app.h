#ifndef __OPENAMP_APP_H__
#define __OPENAMP_APP_H__

#include <rthw.h>
#include <rtthread.h>
#include <rt_openamp.h>
#include <openamp_app_notify.h>

enum oepnamp_app_ept
{
    APP_NOTIFY_EPT = 0,
#ifdef AMP_DEMO_ECHO
    APP_ECHO_EPT,
#endif

#ifdef AMP_DEMO_PTY
    APP_VUART_EPT,
#endif

#ifdef AMP_DEMO_MATRIX
    APP_MATRIX_EPT,
#endif
    APP_NUM,
};

void openamp_app_notify_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest);

#ifdef AMP_DEMO_ECHO
void openamp_app_echo_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest);
#endif

#ifdef AMP_DEMO_PTY
void openamp_app_vuart_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest);
#endif

#ifdef AMP_DEMO_MATRIX
void openamp_app_matrix_multiply_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest);
#endif

#endif /* __OPENAMP_APP_H__ */