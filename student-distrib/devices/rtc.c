/* rtc.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "rtc.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"

void rtc_init(){

  /* Populate IDT entry for rtc */
  set_int_gate(0xA1, rtc_handler_main);

  /* Turn on RTC interrupts */
	cli();
	outb(0x8B, RTC_REG_PORT);		// select register B and disable NMI
	char prev = inb(RW_CMOS_PORT);	// read the current value of register B
	outb(0x8B, RTC_REG_PORT);		// reset the register to B again
	outb(prev | 0x40, RW_CMOS_PORT);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
	sti();
}

void rtc_handler_main(){
  disable_irq(RTC_IRQ_NUM);
	send_eoi(RTC_IRQ_NUM);

  test_interrupts();

  enable_irq(RTC_IRQ_NUM);
}
