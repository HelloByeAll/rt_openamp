#include "openamp_app.h"
#include "rtdbg.h"

#ifdef AMP_DEMO_ENABLE

typedef  void (*app_init_func_t)(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest);

static rt_openamp_t app;
static struct rpmsg_endpoint ept[APP_NUM];

static app_init_func_t app_init_func[APP_NUM] = {
    openamp_app_notify_init,
#ifdef AMP_DEMO_ECHO
    openamp_app_echo_init,
#endif

#ifdef AMP_DEMO_PTY
    openamp_app_vuart_init,
#endif

#ifdef AMP_DEMO_MATRIX
    openamp_app_matrix_multiply_init,
#endif
};

static const char app_name[APP_NUM][24] = {
    "onoff",
#ifdef AMP_DEMO_ECHO
    "echo",
#endif

#ifdef AMP_DEMO_PTY
    "pty",
#endif

#ifdef AMP_DEMO_MATRIX
    "matrix",
#endif
};

static void app_ns_bind_cb(struct rpmsg_device* rdev, const char* name, uint32_t dest)
{
    LOG_W("openamp_ns_bind_cb dest_addr = 0x%X name = %s", dest, name);
#ifdef RT_AMP_MASTER
    rt_uint32_t i;

    for (i = 0; i < APP_NUM; i++)
    {
        if (!rt_strcmp(name, app_name[i]))
        {
            app_init_func[i](&app, &ept[i], app_name[i], RPMSG_ADDR_ANY, dest);
        }
    }
#endif /* RT_AMP_MASTER */
}

static int openamp_app_init(void)
{
    rt_uint32_t i;
    RT_UNUSED(i);

    app.vring_tx   = AMP_DEMO_VRING_TX_ADDR;
    app.vring_rx   = AMP_DEMO_VRING_RX_ADDR;
    app.vring_buf  = AMP_DEMO_VRING_BUF_ADDR;
    app.vring_size = AMP_DEMO_VRING_SIZE;
    app.tx_num     = AMP_DEMO_VRING_TX_DES_NUM;
    app.rx_num     = AMP_DEMO_VRING_RX_DES_NUM;
    app.sgi_no     = AMP_CPU_SGI_INTERRUPT_NO;
    app.ns_bind_cb = app_ns_bind_cb;
    openamp_app_dev_create(&app, "app_rtt");

#ifdef RT_AMP_SLAVE
    for (i = 0; i < APP_NUM; i++)
    {
        app_init_func[i](&app, &ept[i], app_name[i], RPMSG_ADDR_ANY, RPMSG_ADDR_ANY);
    }
#endif /* RT_AMP_SLAVE */

    return 0;
}
INIT_APP_EXPORT(openamp_app_init);
#endif /* AMP_DEMO_ENABLE */
