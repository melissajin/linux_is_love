#ifndef _ISR_H_
#define _ISR_H_

#include "types.h"

//function that initiaztes the first 32 isrs of IDT
void isrs_install();

void add_irq(uint32_t irq, uint32_t handler_addr);

#endif
