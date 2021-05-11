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
#include "sd_card.h"
#include "eth_que.h"


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

/**
  * @brief  Toggle 2 Leds
  * @param  Pin: Gpio Numbers.
  * @retval None
  */
void GpioPinToggle(u32 Pin)
{
	static u8 ledState[2] = {0,0};

	if(ledState[Pin&0x1] == 0)
	{
		XGpioPs_WritePin(&GPIO_LED, Pin, 0x0);
		ledState[Pin&0x1] = 1;
	}
	else
	{
		XGpioPs_WritePin(&GPIO_LED, Pin, 0x1);
		ledState[Pin&0x1] = 0;
	}

}

void toggle_LD4(void)
{
	static u8 ledState = 0;

	if(ledState == 0)
	{
		XGpioPs_WritePin(&GPIO_LED, 7, 0x0);
		ledState = 1;
	}
	else
	{
		XGpioPs_WritePin(&GPIO_LED, 7, 0x1);
		ledState = 0;
	}
}


int main(void)
{
	init_platform();
	init_Interrupt();
	init_ETH_UDP();
	init_GPIO();

	/* now enable interrupts */
	platform_enable_interrupts();

	uint16_t rxEthData[1200];
	uint16_t rxEthLen = 0;

	while (1) {
		loop_ETH_UDP();

		if(timer_1msFlag == TRUE)
		{
			timer_1msFlag = FALSE;
			toggle_LD4();
		}

		rxEthLen = getEthQue((uint8_t *)rxEthData);
		if(rxEthLen != 0)
		{
			/***************** DEBUG ******************
			xil_printf("\r\n");
			for(int i=0; i<rxEthLen; i++)
			{
				xil_printf("%4x ", rxEthData[i]);
			}
			******************************************/

			if ((rxEthData[0] == 0x5453) && (rxEthData[1] == 0x5241))
			{
				xil_printf("ETH Client Connected.\r\n");
				char tmpString[98];
				int strLen = 0;
				strLen = sprintf(tmpString, " TRCB TEST EQUIPEMENT. Ready. V%d.%d\r\n",VERSION_MAJOR, VERSION_MINOR);
				uint16_t tmpSendData[50];
				tmpSendData[0] = 0x5354;
				memcpy((void *)&tmpSendData[1], tmpString, strLen);
				send_UDPData((uint8_t *)tmpSendData, strLen + 2);
			}

			if ((rxEthData[0] == 0xAAAA) && (rxEthData[1] == 0x0A08))
			{
				uint16_t tmpSendData[4] = {0x7E7E, 0x0A04, 0x0000, 0x0000};
				if(write_BootImage((uint16_t *)rxEthData) == XST_SUCCESS)
				{
					tmpSendData[2] = 1;
				}
				else
				{
					tmpSendData[2] = 0;
				}
				send_UDPData((uint8_t *)tmpSendData, sizeof(tmpSendData));
			}

			if ((rxEthData[0] == 0x7E7E) && (rxEthData[1] == 0x0A08))
			{
				uint16_t tmpSendData[4] = {0x7E7E, 0x0A04, 0x0001, 0x0000};
				send_UDPData((uint8_t *)tmpSendData, sizeof(tmpSendData));
			}

			rxEthLen = 0;
		}
	}

	/* never reached */
	cleanup_platform();

	return 0;
}
