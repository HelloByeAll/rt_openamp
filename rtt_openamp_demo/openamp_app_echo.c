
#include "openamp_app.h"

#define DBG_LEVEL         DBG_LOG
#include "rtdbg.h"

#ifdef AMP_DEMO_ECHO
static struct rpmsg_endpoint* app_ept;

static int rpmsg_endpoint_cb(struct rpmsg_endpoint* ept, void* data,
    size_t len, uint32_t src, void* priv)
{
    ((rt_uint8_t *) data)[len] = 0;
#ifdef RT_AMP_MASTER
    LOG_I("cpu#%d rpmsg_endpoint_cb  src:0x%X, len:%d, data:%s", rt_hw_cpu_id(), src, len, data);
#else
    LOG_I("cpu#%d receive msg len:%d, data:%s", rt_hw_cpu_id(), len, data);
    rpmsg_send(ept, data, len);
#endif /* RT_AMP_MASTER */

    return 0;
}

static void app_ns_unbind_cb(struct rpmsg_endpoint* ept)
{
    LOG_I("app_ns_unbind_cb ept addr = 0x%X  dest_addr = 0x%X name = %s",
            ept->addr, ept->dest_addr, ept->name);
}

void openamp_app_echo_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest)
{
    rt_uint32_t status;

    app_ept = ept;
    status = openamp_app_create_endpoint(ept, &app->rvdev.rdev, name, src, dest, rpmsg_endpoint_cb, app_ns_unbind_cb);
    if (status)
    {
        LOG_E("openamp_app_create_endpoint = 0x%X", status);
    }
}

#ifdef RT_AMP_MASTER
void app_echo_send(int argc, char* argv[])
{
    rpmsg_send(app_ept, argv[1], rt_strlen(argv[1]));
}
MSH_CMD_EXPORT(app_echo_send, app_echo_send);
#endif /* RT_AMP_MASTER */
#endif /* AMP_DEMO_ECHO */