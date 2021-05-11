#include "interrupt.h"

#define GIC_DEVICE_ID				XPAR_PS7_SCUGIC_0_DEVICE_ID

uint64_t tick_1ms = 0;
int timer_1msFlag = FALSE;
int timer_2msFlag = FALSE;
static XScuGic gic_Inst;

void IntrHandler_PL_1MS(void *CallBackRef);
int SetupInterruptController(void);

int init_Interrupt(void)
{
	int Status;

	//Setup and Initialize interrupt service
	Status = SetupInterruptController();
	if(Status!=XST_SUCCESS){
		xil_printf("InterruptControllerInit failed! \r\n");
		return XST_FAILURE;
	}
	else xil_printf("Interrupt controller started.\r\n");

	return XST_SUCCESS;
}


int SetupInterruptController(void)
{

	int Status;

	u8 Priority=2, Trigger=0x03;
	XScuGic_Config *IntcConfig; //Interrupt Controller configuration parameters

	//Interrupt Controller initialize
	IntcConfig=XScuGic_LookupConfig(GIC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status=XScuGic_CfgInitialize(&gic_Inst, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if(Status!=XST_SUCCESS){
		return XST_FAILURE;
	}



	//Interrupt setup
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				&gic_Inst);
	Xil_ExceptionEnable();




	//Connect GPIO interrupt to handler
	Status=XScuGic_Connect(&gic_Inst, XPAR_FABRIC_FIT_TIMER_0_INTERRUPT_INTR,
			IntrHandler_PL_1MS, NULL);

	if(Status!=XST_SUCCESS){
		xil_printf("XScuGic_Connect failed! \r\n");
		return XST_FAILURE;
	}

	Priority=0x03, Trigger=0x03;

	XScuGic_SetPriorityTriggerType(&gic_Inst, XPAR_FABRIC_FIT_TIMER_0_INTERRUPT_INTR, Priority, Trigger);
	XScuGic_GetPriorityTriggerType(&gic_Inst, XPAR_FABRIC_FIT_TIMER_0_INTERRUPT_INTR, &Priority, &Trigger);
	xil_printf("InterruptID %d Priority:%d, Trigger:%d\r\n", XPAR_FABRIC_FIT_TIMER_0_INTERRUPT_INTR, Priority, Trigger);

	//Enable interrupt in ARM core
	XScuGic_Enable(&gic_Inst, XPAR_FABRIC_FIT_TIMER_0_INTERRUPT_INTR);


	return XST_SUCCESS;
}

void IntrHandler_PL_1MS(void *CallBackRef)
{
	static int cnt_1sTimer = 0;

	cnt_1sTimer++;
	if(cnt_1sTimer == 1000)
	{
		timer_1msFlag = TRUE;
		cnt_1sTimer = 0;
	}

	if((tick_1ms & 0x1) == 0x1)
	{
		timer_2msFlag = TRUE;
	}

	tick_1ms++;
}

