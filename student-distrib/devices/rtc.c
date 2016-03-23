/* rtc.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "rtc.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"

#define RTC_REG_PORT 0x70  /* Port for specifying reg and disabling NMI */
#define RW_CMOS_PORT 0x71  /* Port used to read from or write to CMOS */

#define NMI_DISABLE  0x80
#define REG_A	     0xA
#define REG_B	     0xB
#define REG_C	     0xC

#define INT_FLAG     0x40

#define RATE_DEFAULT 0xF
#define RATE_MAX     1024
#define RTC_ABS_MAX  32768

extern void rtc_isr();

static int reading, open = 0;

void rtc_init() {
    /* Populate IDT entry for rtc */
    set_int_gate(0x28, (unsigned long) rtc_isr);    
}

void rtc_handler_main() {
 
  test_interrupts();
  // Reset the C register to get the next interrupt
  outb(REG_C, RTC_REG_PORT);
  inb(RW_CMOS_PORT);

  reading = 0;

  send_eoi(RTC_IRQ_NUM);
}

/* open rtc */
int32_t rtc_open(const uint8_t * filename) {
    char curr;

    if(!open) {
        /* Turn on RTC interrupts */
        cli();  /* don't interrupt so rtc is not left in undefined state */
        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);  /* select register B and disable NMI */
        curr = inb(RW_CMOS_PORT);  /* read the current value of register B */
        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);  /* reset the register to B again */
        outb(curr | INT_FLAG, RW_CMOS_PORT);  /* turn on bit 6 of register B */
        sti();

        enable_irq(RTC_IRQ_NUM);
        open = 1;
    }

    /* default rate to 2 Hz */
    cli();
    outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
    curr = inb(RW_CMOS_PORT);
    outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
    outb((curr & 0xF0) | RATE_DEFAULT, RW_CMOS_PORT);  /* 0xF0 - take top 4 bits of curr */
    sti();

    return 0;
}

/* wait until next rtc interrupt */
int32_t rtc_read(int32_t fd, void * buf, int32_t nbytes) {
    reading = 1;
    while(reading);  /* spin until an interrupt happens */
    reading = 1;
    return 0;
}

/* change rtc frequency */
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes) {
    int32_t rate, ratefactor;
    char curr, rs = 0;  /* rate select */

    /* check validity of buffer */
    if(buf == NULL) return -1;

    rate = *((int32_t *) buf);

    /* make sure rate is in range */
    if(rate == 0 || rate > RATE_MAX)
        return -1;

    /* sanity check for power of 2 */
    /* from http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/ */ 
    if(rate & (rate - 1))  /* will only be 0 if rate is a power of 2 */
        return -1;

    ratefactor = RTC_ABS_MAX / rate;
    /* calculate log_2(ratefactor) + 1 */
    while(ratefactor >>= 1) {
        rs++;
    }
    rs++;

    cli();
    outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
    curr = inb(RW_CMOS_PORT);
    outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
    outb((curr & 0xF0) | rs, RW_CMOS_PORT);
    sti();

    return 0;
}

/* close rtc */
int32_t rtc_close(int32_t fd) {
    char curr;
    disable_irq(RTC_IRQ_NUM);

    /* turn off RTC interrupts */
    cli();
    outb(NMI_DISABLE | REG_B, RTC_REG_PORT);
    curr = inb(RW_CMOS_PORT);
    outb(NMI_DISABLE | REG_B, RTC_REG_PORT);
    outb(curr & ~INT_FLAG, RW_CMOS_PORT);  /* turn off bit 6 of register B */
    sti();

    open = 0;

    return 0;
}
