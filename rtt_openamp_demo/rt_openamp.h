#ifndef __RT_OPENAMP_H__
#define __RT_OPENAMP_H__

#include <rtthread.h>
#include "metal/sys.h"
#include "metal/device.h"
#include "openamp/open_amp.h"
#include "openamp/virtio.h"

struct rt_openamp
{
    struct rpmsg_virtio_device rvdev;
    struct rpmsg_endpoint* ept;
    rpmsg_ns_bind_cb ns_bind_cb;
    rt_sem_t sem;
    rt_ubase_t vring_tx;
    rt_uint32_t tx_num;
    rt_ubase_t vring_rx;
    rt_uint32_t rx_num;
    rt_ubase_t vring_buf;
    rt_uint32_t vring_size;
    rt_uint32_t sgi_no;
    void* user_data;
};
typedef struct rt_openamp rt_openamp_t;

struct metal_io_region* openamp_shmemio_get(void);

int openamp_app_dev_create(struct rt_openamp *app, char *name);

void openamp_app_ept_delete(struct rpmsg_endpoint *ept);
int openamp_app_create_endpoint(
    struct rpmsg_endpoint* ept, struct rpmsg_device* rdev,
    const char* name, uint32_t src, uint32_t dest,
    rpmsg_ept_cb cb, rpmsg_ns_unbind_cb unbind_cb);

#endif /* __RT_OPENAMP_H__ */