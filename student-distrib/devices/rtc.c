/* rtc.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "rtc.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"

extern void rtc_isr();

void rtc_init(){

  /* Populate IDT entry for rtc */
  set_int_gate(0x28, (unsigned long) rtc_isr);

  /* Turn on RTC interrupts */
	outb(0x8B, RTC_REG_PORT);		// select register B and disable NMI
	char curr = inb(RW_CMOS_PORT);	// read the current value of register B
	outb(0x8B, RTC_REG_PORT);		// reset the register to B again
	outb(curr | 0x40, RW_CMOS_PORT);

  /* Change the rate of interrupts */
  char rate = 0x0f; // Rate has to be between 2 and 15
  outb(0x8A, RTC_REG_PORT);
  curr = inb(RW_CMOS_PORT);
  outb(0x8A, RTC_REG_PORT);
  outb(0x71, (curr & 0xF0) | rate);

  enable_irq(RTC_IRQ_NUM);
}

void rtc_handler_main(){
 
  test_interrupts();
  // Reset the C register to get the next interrupt
  outb(0x0C, RTC_REG_PORT);
  inb(RW_CMOS_PORT);

  send_eoi(RTC_IRQ_NUM);
}
