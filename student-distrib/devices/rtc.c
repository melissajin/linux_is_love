/* rtc.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "rtc.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"

#define NMI_DISABLE  0x80
#define REG_A	     0xA
#define REG_B	     0xB
#define REG_C	     0xC

#define RATE_DEFAULT 0xF
#define RATE_MAX     1024
#define RTC_ABS_MAX  32768

extern void rtc_isr();

static int reading, open = 0;

void rtc_init(){
    /* Populate IDT entry for rtc */
    set_int_gate(0x28, (unsigned long) rtc_isr);    
}

void rtc_handler_main(){
 
  test_interrupts();
  // Reset the C register to get the next interrupt
  outb(REG_C, RTC_REG_PORT);
  inb(RW_CMOS_PORT);

  reading = 0;

  send_eoi(RTC_IRQ_NUM);
}

/* open rtc */
unsigned long rtc_open() {
    char curr;
    if(!open) {
        /* Turn on RTC interrupts */
        cli();  /* don't interrupt so rtc is not let in undefined state */
        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);    // select register B and disable NMI
        curr = inb(RW_CMOS_PORT);           // read the current value of register B
        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);    // reset the register to B again
        outb(curr | 0x40, RW_CMOS_PORT);        // turn on bit 6 of register B
        sti();
        enable_irq(RTC_IRQ_NUM);
        open = 1;
    }
    return 0;
}

/* wait until next rtc interrupt */
unsigned long rtc_read() {
    reading = 1;
    while(reading);  /* spin until an interrupt happens */
    reading = 1;
    return 0;
}

/* change rtc frequency */
unsigned long rtc_write(unsigned long rate) {
    unsigned long ratefactor;
    char curr, rs = 0;  /* rate select */
    /* make sure rate is in range */
    if(rate == 0 || rate > RATE_MAX)
        return -1;
    /* sanity check for power of 2 */
    /* from http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/ */ 
    if(rate & (rate - 1))  /* will only be 0 if rate is a power of 2 */
        return -1;

    ratefactor = RTC_ABS_MAX / rate;
    while(ratefactor >>= 1) {
        rs++;
    }
    rs++;

    cli();
    outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
    curr = inb(RW_CMOS_PORT);
    outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
    outb((curr & 0xF0) | rs, RW_CMOS_PORT);       //0xF0 a mask to take top 4 bits
    sti();

    return 0;
}

/* close rtc */
unsigned long rtc_close() {
    char curr;
    disable_irq(RTC_IRQ_NUM);
    open = 0;

    /* turn off RTC interrupts */
    cli();
    outb(NMI_DISABLE | REG_B, RTC_REG_PORT);
    curr = inb(RW_CMOS_PORT);
    outb(NMI_DISABLE | REG_B, RTC_REG_PORT);
    outb(curr & 0xBF, RW_CMOS_PORT);  /* turn off bit 6 of register B */
    sti();

    return 0;
}
