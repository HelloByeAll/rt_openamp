#include <rthw.h>
#include "metal/irq.h"

unsigned int metal_irq_save_disable(void)
{
    return rt_hw_interrupt_disable();
}

void metal_irq_restore_enable(unsigned int flags)
{
    rt_hw_interrupt_enable(flags);
}
