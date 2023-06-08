
#include <rtthread.h>
#include "metal/sys.h"
#include "metal/device.h"
#include "openamp/open_amp.h"

#define SHM_DEVICE_NAME     "rtshm"

#define DEFAULT_PAGE_SHIFT   (-1U)
#define DEFAULT_PAGE_MASK    (-1U)

#ifndef SHM_START_ADDR
#define SHM_START_ADDR      AMP_SHARE_MEMORY_ADDRESS
#endif /* SHM_START_ADDR */

#ifndef SHM_SIZE
#define SHM_SIZE            AMP_SHARE_MEMORY_SIZE
#endif /* SHM_SIZE */

static struct metal_io_region* shm_io;
static struct metal_device shm_device = {
    .name = SHM_DEVICE_NAME,
    .bus = NULL,
    .num_regions = 1,
    .node = { NULL },
    .irq_num = 0,
    .irq_info = NULL
};

static int openamp_shmem_init(void)
{
    int status = 0;
    struct metal_device* device;
    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    static metal_phys_addr_t shm_physmap;

    metal_init(&metal_params);

    status = metal_register_generic_device(&shm_device);
    if (status != 0)
    {
        return status;
    }

    status = metal_device_open("generic", SHM_DEVICE_NAME, &device);
    if (status != 0)
    {
        return status;
    }

    shm_physmap = SHM_START_ADDR;
    metal_io_init(&device->regions[0], (void*)SHM_START_ADDR, &shm_physmap,
                    SHM_SIZE, DEFAULT_PAGE_SHIFT, 0, NULL);

    shm_io = metal_device_io_region(device, 0);
    if (shm_io == NULL)
    {
        return -1;
    }

    return 0;
}
INIT_ENV_EXPORT(openamp_shmem_init);

void openamp_shmem_deinit(void)
{
    metal_finish();
}

struct metal_io_region* openamp_shmemio_get(void)
{
    return shm_io;
}
