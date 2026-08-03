#ifndef __RTC_H
#define __RTC_H

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
void rtc_init(void);
void rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec);
void rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week);
void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week);
void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm);
#endif 
