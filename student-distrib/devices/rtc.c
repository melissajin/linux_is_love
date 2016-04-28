/* rtc.c - Functions used in interfacing the keyboard with PIC interrupt
 *
 */

#include "rtc.h"
#include "../i8259.h"
#include "../isr.h"
#include "../lib.h"
#include "../fs.h"
#include "../process.h"
#include "keyboard.h"

#define RTC_REG_PORT 0x70  /* Port for specifying reg and disabling NMI */
#define RW_CMOS_PORT 0x71  /* Port used to read from or write to CMOS */

#define NMI_DISABLE  0x80
#define REG_A	     0xA
#define REG_B	     0xB
#define REG_C	     0xC

#define INT_FLAG     0x40

#define RATE_DEFAULT 0xF
#define RATE_MIN     2
#define RATE_MAX     1024
#define RTC_ABS_MAX  32768

typedef struct {
    int32_t rate;
    int32_t curr_count;
    int32_t max_count;
} rtc_process;

static int32_t rtc_open(const uint8_t * filename);
static int32_t rtc_read(int32_t fd, void * buf, int32_t nbytes);
static int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes);
static int32_t rtc_close(int32_t fd);

static void set_rate(int32_t rate);
static void update_rtc_processes();

static int32_t rtc_rate;
static int open = 0;
static rtc_process procs[MAX_PROCESSES];

static fops_t rtc_fops = {
    .read = rtc_read,
    .write = rtc_write,
    .open = rtc_open,
    .close = rtc_close
};

void rtc_init() {
    int i;

    /* Populate IDT entry for rtc */
    add_irq(RTC_IRQ_NUM, (uint32_t) rtc_handler_main);

    add_device(RTC_FTYPE, &rtc_fops);

    for(i = 0; i < MAX_PROCESSES; i++) {
        procs[i].rate = RATE_MIN;
    }
}

void rtc_handler_main() {
    int i;

    //test_interrupts();
    // Reset the C register to get the next interrupt
    send_eoi(RTC_IRQ_NUM);
    outb(REG_C, RTC_REG_PORT);
    inb(RW_CMOS_PORT);

    for(i = 0; i < MAX_PROCESSES; i++) procs[i].curr_count--;
}

/* open rtc */
int32_t rtc_open(const uint8_t * filename) {
    uint8_t curr;

    if(!open) {
        /* Turn on RTC interrupts */
        cli();  /* don't interrupt so rtc is not left in undefined state */

        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);  /* select register B and disable NMI */
        curr = inb(RW_CMOS_PORT);  /* read the current value of register B */
        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);  /* reset the register to B again */
        outb(curr | INT_FLAG, RW_CMOS_PORT);  /* turn on bit 6 of register B */

        /* default rate to 2 Hz */
        outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
        curr = inb(RW_CMOS_PORT);
        outb(NMI_DISABLE | REG_A, RTC_REG_PORT);
        outb((curr & 0xF0) | RATE_DEFAULT, RW_CMOS_PORT);  /* 0xF0 - take top 4 bits of curr */

        rtc_rate = RATE_MIN;

        sti();

        enable_irq(RTC_IRQ_NUM);
    }

    open++;

    return 0;
}

/* wait until next rtc interrupt */
int32_t rtc_read(int32_t fd, void * buf, int32_t nbytes) {
    pcb_t * pcb;
    int32_t pid;;

    pcb(pcb);
    pid = pcb -> pid;

    procs[pid - 1].curr_count = procs[pid - 1].max_count;
    while(procs[pid - 1].curr_count > 0);  /* spin until enough interrupts happen */

    return 0;
}

/* change rtc frequency */
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes) {
    int32_t rate;
    pcb_t * pcb;

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

    pcb(pcb);
    procs[pcb -> pid - 1].rate = rate;
    procs[pcb -> pid - 1].max_count = rtc_rate / rate;

    if(rate > rtc_rate) {
        rtc_rate = rate;
        set_rate(rate);
        update_rtc_processes();
    }

    return 1;
}

/* close rtc */
int32_t rtc_close(int32_t fd) {
    uint8_t curr;
    int i;
    pcb_t * pcb;

    pcb(pcb);

    procs[pcb -> pid - 1].rate = RATE_MIN;

    open--;

    if(!open) {
        disable_irq(RTC_IRQ_NUM);

        /* turn off RTC interrupts */
        cli();
        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);
        curr = inb(RW_CMOS_PORT);
        outb(NMI_DISABLE | REG_B, RTC_REG_PORT);
        outb(curr & ~INT_FLAG, RW_CMOS_PORT);  /* turn off bit 6 of register B */
        sti();
    } else {
        /* "trim" rtc_rate if necessary */
        rtc_rate = RATE_MIN;
        for(i = 0; i < MAX_PROCESSES; i++) {
            if(procs[i].rate > rtc_rate) rtc_rate = procs[i].rate;
        }
        update_rtc_processes();
    }

    return 0;
}

void set_rate(int32_t rate) {
    int32_t ratefactor;
    uint8_t curr, rs = 0;  /* rate select */

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
}

void update_rtc_processes() {
    int i;
    for(i = 0; i < MAX_PROCESSES; i++) {
        procs[i].max_count = rtc_rate / procs[i].rate;
    }
}
