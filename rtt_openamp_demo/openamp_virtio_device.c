
#include <rthw.h>
#include <rtthread.h>
#include <interrupt.h>
#include <rt_openamp.h>

#define DBG_TAG                     "openamp"
#define DBG_LVL                     DBG_INFO
#include <rtdbg.h>

#define VDEV_STATUS_ADDR    AMP_VDEV_STATUS_ADDRESS

static int ipi_remote_cpu(void)
{
#ifdef RT_AMP_MASTER
    return AMP_SLAVE_CPU_NO;
#else
    return AMP_MASTRT_CPU_NO;
#endif
}

static int openamp_app_role_get(void)
{
#ifdef RT_AMP_MASTER
    return RPMSG_HOST;
#else
    return RPMSG_REMOTE;
#endif
}

static unsigned char virtio_get_status(struct virtio_device* vdev)
{
    return HWREG8(VDEV_STATUS_ADDR);
}

static void virtio_set_status(struct virtio_device* vdev, unsigned char status)
{
    HWREG8(VDEV_STATUS_ADDR) = status;
}

static uint32_t virtio_get_features(struct virtio_device* vdev)
{
    return (1UL << (VIRTIO_RPMSG_F_NS));
}

static void virtio_set_features(struct virtio_device* vdev, uint32_t features)
{
}

static void virtio_notify(struct virtqueue* vq)
{
    struct rpmsg_virtio_device *rvdev;
    struct virtio_device *vdev;
    struct rt_openamp *app;
    static rt_uint32_t cpu_mask;

    vdev = vq->vq_dev;
    rvdev = vdev->priv;
    app = (struct rt_openamp *) rvdev;

    /* call SGI to generate notify */
    // LOG_I("send to CPU#%d openamp_notify ...SGI(%d)", ipi_remote_cpu(), app->sgi_no);

    cpu_mask = 1 << ipi_remote_cpu();
    rt_hw_ipi_send(app->sgi_no, cpu_mask);
}

static const struct virtio_dispatch dispatch = {
    .get_status   = virtio_get_status,
    .set_status   = virtio_set_status,
    .get_features = virtio_get_features,
    .set_features = virtio_set_features,
    .notify       = virtio_notify,
};

static void openamp_sgi_isr_handler(int vector, void* param)
{
    rt_sem_t openamp_sem = (rt_sem_t)param;
    // LOG_W("openamp_sgi_isr_handler CPU#0");
    /* signal the semaphore */
    rt_sem_release(openamp_sem);
}

int openamp_app_create_endpoint(
    struct rpmsg_endpoint* ept, struct rpmsg_device* rdev,
    const char* name, uint32_t src, uint32_t dest,
    rpmsg_ept_cb cb, rpmsg_ns_unbind_cb unbind_cb)
{
#ifdef RT_AMP_MASTER
    return rpmsg_create_ept(ept, rdev, name, src, dest, cb, unbind_cb);
#else
    return rpmsg_create_ept(ept, rdev, name, dest, src, cb, unbind_cb);
#endif
}

static void openamp_thread_entry(void* parameter)
{
    struct rt_openamp* app = (struct rt_openamp*)parameter;
    struct virtqueue* rx_vq;

    if (rpmsg_virtio_get_role(&app->rvdev) == RPMSG_HOST)
    {
        rx_vq = app->rvdev.vdev->vrings_info[0].vq;
    }
    else
    {
        rx_vq = app->rvdev.vdev->vrings_info[1].vq;
    }

    while (1)
    {
        /* 等待来自SGI的信号 */
        rt_sem_take(app->sem, RT_WAITING_FOREVER);

        /* openamp的virtual queue处理, 在任务上下午调用endpoint cb */
        virtqueue_notification(rx_vq);
    }
}

static void openamp_ns_bind_cb(struct rpmsg_device* rdev, const char* name, uint32_t dest)
{
    LOG_I("openamp_ns_bind_cb dest_addr = 0x%X name = %s", dest, name);
}

int openamp_app_dev_create(struct rt_openamp* app, char* name)
{
    struct metal_io_region* shm_io;
    struct rpmsg_virtio_shm_pool* shpool;
    struct virtio_device* vdev;
    struct virtio_vring_info* vrings;
    struct virtqueue* vq[2];
    rt_thread_t tid;

    shm_io = openamp_shmemio_get();

    app->sem = rt_sem_create(name, 0, RT_IPC_FLAG_FIFO);
    if (app->sem == RT_NULL)
    {
        return -RT_ERROR;
    }

    rt_hw_interrupt_install(app->sgi_no, openamp_sgi_isr_handler, app->sem, "OPENAMP IPI_HANDLER");
    rt_hw_interrupt_umask(app->sgi_no);

    vq[0] = virtqueue_allocate(app->tx_num);
    if (!vq[0])
    {
        LOG_E("virtqueue_allocate failed to alloc vq[0]");
        return -1;
    }

    vq[1] = virtqueue_allocate(app->rx_num);
    if (!vq[1])
    {
        LOG_E("virtqueue_allocate failed to alloc vq[1]");
        return -1;
    }

    vrings                   = rt_malloc(sizeof(struct virtio_vring_info) * 2);
    rt_memset(vrings, 0, sizeof(struct virtio_vring_info) * 2);
    vrings[0].io             = shm_io;
    vrings[0].info.vaddr     = (void*)(app->vring_tx);
    vrings[0].info.num_descs = app->tx_num;
    vrings[0].info.align     = 4;
    vrings[0].vq             = vq[0];

    vrings[1].io             = shm_io;
    vrings[1].info.vaddr     = (void*)app->vring_rx;
    vrings[1].info.num_descs = app->rx_num;
    vrings[1].info.align     = 4;
    vrings[1].vq             = vq[1];

    vdev                     = rt_malloc(sizeof(struct virtio_device));
    vdev->role               = openamp_app_role_get();
    vdev->vrings_num         = 2;
    vdev->func               = &dispatch;
    vdev->vrings_info        = vrings;

    shpool = rt_malloc(sizeof(struct rpmsg_virtio_shm_pool));
    rpmsg_virtio_init_shm_pool(shpool, (void*)app->vring_buf, (size_t)app->vring_size);
    if (app->ns_bind_cb)
    {
        rpmsg_init_vdev(&app->rvdev, vdev, app->ns_bind_cb, shm_io, shpool);
    }
    else
    {
        rpmsg_init_vdev(&app->rvdev, vdev, openamp_ns_bind_cb, shm_io, shpool);
    }

    tid = rt_thread_create(name, openamp_thread_entry, app, 8192, 5, 10);
    if (tid == RT_NULL)
    {
        LOG_E("openamp thread create failed!");
        return -RT_ERROR;
    }

    rt_thread_startup(tid);

    return 0;
}

void openamp_app_ept_delete(struct rpmsg_endpoint* ept)
{
    rpmsg_destroy_ept(ept);
}