#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include "./SYSTEM/sys/sys.h"
#include <stdbool.h>

void bsp_timer_init(void);
bool bsp_timer_sample_due(void);

#endif


