#ifndef __TIMELIB__
#define __TIMELIB__

#include <stdint.h>
#include "StrLib.h"
#include "NumberBaseLib.h"

extern int isLeapYear(uint32_t year);
extern uint32_t get_timestamp(uint32_t NowYear, uint32_t NowMonth, uint32_t NowDay, uint32_t NowHour,
                              uint32_t NowMinute, uint32_t NowSecond);
extern uint32_t getTimeNumber_UTCByRTCTime(strnew RTCTime_String);
typedef struct _TimeStuClass {
    uint32_t year;   /** 年 */
    uint32_t month;  /** 月 */
    uint32_t day;    /** 日 */
    uint32_t week;   /** 周 */
    uint32_t hour;   /** 时 */
    uint32_t minute; /** 分 */
    uint32_t second; /** 秒 */
} TimeStuClass;
extern TimeStuClass timestampToRTCData(uint32_t timestamp);
extern int getDayOfWeek(uint32_t iYear, uint32_t iMonth, uint32_t iDay);

#endif
