/* rtc.h - Defines the functions used for the real time clock
 *
 */

#ifndef _RTC_H
#define _RTC_H

#include "../types.h"
 
#define RTC_IRQ_NUM  8

/* call to initialize rtc by adding handler to IDT */
void rtc_init();
/* rtc interrupt handler */
void rtc_handler_main();

/* replace these with fops struct later */
/* open rtc */
int32_t rtc_open(const uint8_t * filename);

/* wait until next rtc interrupt */
int32_t rtc_read(int32_t fd, void * buf, int32_t nbytes);

/* change rtc frequency */
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes);

/* close rtc */
int32_t rtc_close(int32_t fd);

#endif
