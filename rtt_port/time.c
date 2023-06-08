#include <rtthread.h>
#include "metal/time.h"

unsigned long long metal_get_timestamp(void)
{
    return rt_tick_get();
}
