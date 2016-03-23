/* rtc.h - Defines the functions used for the real time clock
 *
 */

#ifndef _RTC_H
#define _RTC_H

#include "../types.h"

#define RTC_REG_PORT     0X70 // Port used to specify the register number
                              // and to disable NMI
#define RW_CMOS_PORT     0x71 // Port used to read/write from from CMOS
#define RTC_IRQ_NUM  0x08

//initializes rtc
void rtc_init();
//rtc interrupt
void rtc_handler_main();

/* open rtc */
unsigned long rtc_open();

/* wait until next rtc interrupt */
unsigned long rtc_read();

/* change rtc frequency */
unsigned long rtc_write(unsigned long rate);

/* close rtc */
unsigned long rtc_close();

#endif
