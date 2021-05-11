#ifndef __PLATFORM_H_
#define __PLATFORM_H_
#include "xil_types.h"
#include "xil_printf.h"

#define VERSION_MAJOR 0U
#define VERSION_MINOR 1U

#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#define PLATFORM_ZYNQ 

#define ADDRESS_BRAM_WR_LENGTH           0x40000004
#define ADDRESS_BRAM_WR_START            0x4000000C
#define ADDRESS_BRAM_RD_START            0x4200000C
#define ADDRESS_BRAM_SIG_START           0x4400000C

#define ADDRESS_GUIBRAM_WR_START         0x4800000C
#define ADDRESS_GUIBRAM_RD_START         0x4600000C

#define ADDRESS_BRAM_LUTWRITE       ADDRESS_BRAM_WR_START
#define ADDRESS_BRAM_FWWRITE        ADDRESS_BRAM_WR_START
#define ADDRESS_BRAM_MRAMWRITE      ADDRESS_BRAM_WR_START
#define ADDRESS_BRAM_AGILITYWRITE   ADDRESS_BRAM_WR_START



//MESSAGE Frame
#define MESSAGE_START_CODE1			0x5a
#define MESSAGE_START_CODE2			0xa5
#define MESSAGE_START_CODE_7E       0x7e


void init_platform();
void cleanup_platform();
void platform_setup_timer();
void platform_enable_interrupts();
u64 get_time_ms();

#endif
