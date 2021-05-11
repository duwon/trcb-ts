/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include "xparameters.h"
#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xgpiops.h"

#include "eth_udp.h"
#include "interrupt.h"

static XGpioPs GPIO_LED;

void init_GPIO(void)
{
	XGpioPs_Config *GpioConfigPtr;

	//GPIO Initialization
	GpioConfigPtr = XGpioPs_LookupConfig(0);
	XGpioPs_CfgInitialize(&GPIO_LED, GpioConfigPtr,GpioConfigPtr->BaseAddr);

	//set direction and enable output
	XGpioPs_SetDirectionPin(&GPIO_LED, 7, 1);
	XGpioPs_SetOutputEnablePin(&GPIO_LED, 7, 1);
	XGpioPs_WritePin(&GPIO_LED, 7, 1);

}

int main(void)
{
	init_platform();
	init_GPIO();
	init_Interrupt();
	init_ETH_UDP();

	/* now enable interrupts */
	platform_enable_interrupts();


	while (1) {
		loop_ETH_UDP();

		if(flag_1sTimer == TRUE)
		{
			uint16_t tmpData[3] = {0x7E7E, 0x0010, 0xAAA};
			flag_1sTimer = FALSE;
			send_UDPData((uint8_t *)tmpData, sizeof(tmpData));
		}
	}

	/* never reached */
	cleanup_platform();

	return 0;
}
