/* rtc.h - Defines the functions used for the real time clock
 *
 */

#ifndef _RTC_H
#define _RTC_H

#include "../types.h"
#include "../fs.h"
 
#define RTC_IRQ_NUM  8

/* call to initialize rtc by adding handler to IDT */
void rtc_init();
/* rtc interrupt handler */
void rtc_handler_main();

fops_t rtc_fops;

#endif
