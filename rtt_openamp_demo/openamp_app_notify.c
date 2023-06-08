
#include "openamp_app.h"
#include "rtdbg.h"

static struct rpmsg_endpoint* app_ept;

static int rpmsg_endpoint_cb(struct rpmsg_endpoint* ept, void* data,
    size_t len, uint32_t src, void* priv)
{
    rt_uint32_t* cmd = data;

#ifdef RT_AMP_MASTER
#else
#ifdef AMP_DEMO_PTY
    extern void openamp_pty_open(void);
    extern void openamp_pty_close(void);
#endif /* AMP_DEMO_PTY */

    switch (*cmd)
    {
#ifdef AMP_DEMO_PTY
    case M2D_PTY_OPEN:
        openamp_pty_open();
        break;

    case M2D_PTY_CLOSE:
        openamp_pty_close();
        break;
#endif /* AMP_DEMO_PTY */

    default:
        break;
    }

#endif /* RT_AMP_MASTER */

    return 0;
}

void openamp_app_notify_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest)
{
    int status;

    app_ept = ept;
    status = openamp_app_create_endpoint(ept, &app->rvdev.rdev, name, src, dest, rpmsg_endpoint_cb, RT_NULL);
    if (status)
    {
        LOG_E("openamp_app_create_endpoint = 0x%X", status);
    }
}

void openamp_app_notify_cmd(rt_uint32_t cmd)
{
    rt_uint8_t buf[4];

    rt_memcpy(buf, &cmd, 4);
    rpmsg_send(app_ept, buf, 4);
}