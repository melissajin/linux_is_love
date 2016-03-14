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

void rtc_init();

void rtc_handler_main();

#endif
