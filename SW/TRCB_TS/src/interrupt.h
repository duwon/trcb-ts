#ifndef SRC_INTERRUPT_H_
#define SRC_INTERRUPT_H_

#include "xscugic.h"
#include "platform.h"
#include "xil_printf.h"


extern int flag_1sTimer;

int init_Interrupt(void);

#endif /* SRC_PL_SYSDEF_H_ */
