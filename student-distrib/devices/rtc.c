/* rtc.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "rtc.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"

#define NMI_DISABLE 0x80
#define REG_A	    0xA
#define REG_B	    0xB
#define REG_C	    0xC



extern void rtc_isr();

void rtc_init(){

	char curr;
	char rate;
  /* Populate IDT entry for rtc */
  set_int_gate(0x28, (unsigned long) rtc_isr);

  /* Turn on RTC interrupts */
	outb(NMI_DISABLE | REG_B, RTC_REG_PORT);	// select register B and disable NMI
	curr = inb(RW_CMOS_PORT);			// read the current value of register B
	outb(NMI_DISABLE | REG_B, RTC_REG_PORT);	// reset the register to B again
	outb(curr | 0x40, RW_CMOS_PORT); 		// turn on bit 6 of register B

  /* Change the rate of interrupts */
  rate = 0x0f; // Rate has to be between 2 and 15
  outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
  curr = inb(RW_CMOS_PORT);
  outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
  outb(RW_CMOS_PORT, (curr & 0xF0) | rate); 		//0xF0 a mask to take top 4 bits 

  enable_irq(RTC_IRQ_NUM);
}

void rtc_handler_main(){
 
  //test_interrupts();
  // Reset the C register to get the next interrupt
  outb(REG_C, RTC_REG_PORT);
  inb(RW_CMOS_PORT);

  send_eoi(RTC_IRQ_NUM);
}
