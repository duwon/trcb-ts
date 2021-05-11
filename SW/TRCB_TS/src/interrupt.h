#ifndef SRC_INTERRUPT_H_
#define SRC_INTERRUPT_H_

#include "xscugic.h"
#include "platform.h"


extern int timer_1msFlag;
extern int timer_2msFlag;
extern uint64_t tick_1ms;
int init_Interrupt(void);

#endif /* SRC_PL_SYSDEF_H_ */
