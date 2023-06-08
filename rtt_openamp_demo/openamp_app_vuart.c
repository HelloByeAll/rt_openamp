#include "openamp_app.h"
#include "rtdbg.h"
#include "vconsole.h"
#include "msh.h"

#ifdef AMP_DEMO_PTY
static struct rpmsg_endpoint* app_ept;
static rt_openamp_t* vapp;
static rt_device_t console[2];

static int rpmsg_endpoint_cb(struct rpmsg_endpoint* ept, void* data,
    size_t len, uint32_t src, void* priv)
{
#ifdef RT_AMP_MASTER
    rt_device_write(console[0], 0, data, len);
#else
    vconsole_input(console[1], data, len);
#endif /* RT_AMP_MASTER */

    return 0;
}

static rt_size_t openamp_vuart_output(rt_device_t device, rt_uint8_t* buff, rt_size_t size)
{
#ifdef RT_AMP_MASTER
    rt_device_write(console[0], 0, buff, size);
#else
    rpmsg_send(app_ept, buff, size);
#endif

    return 0;
}

static void amp_vuart_console_close(void)
{
    rt_device_close(console[0]);
    vconsole_switch(console[0]);

#ifdef RT_AMP_MASTER
    openamp_app_notify_cmd(M2D_PTY_CLOSE);
    msh_exec("version", rt_strlen("version"));
#endif /* RT_AMP_MASTER */
}

#ifdef RT_AMP_MASTER
static rt_sem_t rx_sem = RT_NULL;
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(rx_sem);
    return RT_EOK;
}

static void old_console_thread_entry(void* parameter)
{
    rt_err_t(*old_rx_indicate)(rt_device_t dev, rt_size_t size);
    rt_uint32_t send_len = 0;
    rt_uint32_t size = 0;
    rt_uint8_t send_buf[RPMSG_BUFFER_SIZE];
    rt_uint8_t read_buf[64];
    rt_tick_t tick_ms;
    int i;

    if (rx_sem == RT_NULL)
    {
        rx_sem = rt_sem_create("rx_sem", 0, RT_IPC_FLAG_FIFO);
    }

    old_rx_indicate = console[0]->rx_indicate;
    rt_device_set_rx_indicate(console[0], uart_input);

    while (1)
    {
        size = rt_device_read(console[0], -1, read_buf, sizeof(read_buf));
        if (size)
        {
            tick_ms = rt_tick_get_millisecond();

            if (send_len + size > sizeof(send_buf))
            {
                rpmsg_send(app_ept, send_buf, send_len);
                send_len = 0;
            }
            for (i = 0; i < size; i++)
            {
                if (read_buf[i] == 0x3)
                {
                    rt_memcpy(send_buf + send_len, read_buf, i + 1);
                    send_len += i + 1;
                    rpmsg_send(app_ept, send_buf, send_len);
                    rt_device_set_rx_indicate(console[0], old_rx_indicate);
                    amp_vuart_console_close();
                    return;
                }
            }

            rt_memcpy(send_buf + send_len, read_buf, size);
            send_len += size;
        }
        else if ((rt_tick_get_millisecond() - tick_ms) > 10)
        {
            if (send_len)
            {
                rpmsg_send(app_ept, send_buf, send_len);
                send_len = 0;
            }

            rt_sem_take(rx_sem, RT_WAITING_FOREVER);
        }
    }
}
#endif

static void amp_vuart_console(void)
{
    rt_device_open(console[0], RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    vconsole_switch(console[1]);

#ifdef RT_AMP_MASTER
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("old_console", old_console_thread_entry, RT_NULL, 8192, 5, 10);
    if (tid)
    {
        rt_thread_startup(tid);
    }
#else
    msh_exec("version", rt_strlen("version"));
#endif /* RT_AMP_MASTER */
}

void openamp_app_vuart_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest)
{
    rt_uint32_t status;

    vapp = app;
    app_ept = ept;
    status = openamp_app_create_endpoint(ept, &app->rvdev.rdev, name, src, dest, rpmsg_endpoint_cb, RT_NULL);
    if (status)
    {
        LOG_E("openamp_app_create_endpoint = 0x%X", status);
    }

    console[0] = rt_console_get_device();
    console[1] = vconsole_create("vuart", openamp_vuart_output);
}

void openamp_pty_open(void)
{
    amp_vuart_console();

#ifdef RT_AMP_MASTER
    openamp_app_notify_cmd(M2D_PTY_OPEN);
#endif
}

void openamp_pty_close(void)
{
    amp_vuart_console_close();

#ifdef RT_AMP_MASTER
    openamp_app_notify_cmd(M2D_PTY_CLOSE);
#endif
}

#ifdef RT_AMP_MASTER
MSH_CMD_EXPORT_ALIAS(openamp_pty_open, app_pty, );
#endif
#endif /* AMP_DEMO_PTY */