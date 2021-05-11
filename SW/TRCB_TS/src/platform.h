#ifndef __PLATFORM_H_
#define __PLATFORM_H_
#include "xil_types.h"

#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#define PLATFORM_ZYNQ 

void init_platform();
void cleanup_platform();
void platform_setup_timer();
void platform_enable_interrupts();
u64 get_time_ms();

#endif
