#include "TimeLib.h"
#include <stdio.h>

// 判断是否为闰年
int isLeapYear(uint32_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
// 计算从1970年1月1日到指定日期的总天数
static int calculate_days(int year, int month, int day) {
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int total_days = 0;
    int i;

    // 计算从1970年到当前年份的总天数
    for (i = 1970; i < year; i++) {
        total_days += (isLeapYear(i) ? 366 : 365);
    }

    // 计算当前年份从1月1日到指定日期的天数
    for (i = 0; i < month - 1; i++) {
        total_days += days_in_month[i];
        if (i == 1 && isLeapYear(year)) { // 2月需要额外加一天(闰年)
            total_days++;
        }
    }
    // 加上当前月份的天数
    total_days += day - 1;
    return total_days;
}
// 计算时间戳(秒数)
uint32_t get_timestamp(uint32_t NowYear, uint32_t NowMonth, uint32_t NowDay, uint32_t NowHour, uint32_t NowMinute,
                       uint32_t NowSecond) {
    // 计算从1970年1月1日到指定日期的总天数
    int days = calculate_days(NowYear, NowMonth, NowDay);
    // 转换为秒数
    long OverTimeSec = days * 86400LL; // 每天86400秒
    OverTimeSec += NowHour * 3600LL;   // 小时转秒
    OverTimeSec += NowMinute * 60LL;   // 分钟转秒
    OverTimeSec += NowSecond;          // 加上秒
    return OverTimeSec;
}
uint32_t getTimeNumber_UTCByRTCTime(strnew RTCTime_String) {
    // RTCTime_String.Name._char 格式为 "YYYY-MM-DD HH:MM:SS"
    int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
    sscanf(RTCTime_String.Name._char, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
    // 计算自1970-01-01 00:00:00以来的秒数(UTC时间戳)
    static const int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint32_t days = 0;

    // 计算从1970到当前年份的天数
    for (int y = 1970; y < year; y++) {
        days += 365;
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
            days += 1; // 闰年
    }

    // 计算当前年内已过的月份天数
    for (int m = 1; m < month; m++) {
        days += days_in_month[m - 1];
        if (m == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
            days += 1; // 当前年闰年2月
    }

    days += (day - 1);

    uint32_t timestamp = days * 86400 + hour * 3600 + min * 60 + sec;
    return timestamp;
}
// 将时间戳转换为时间结构体
TimeStuClass timestampToRTCData(uint32_t timestamp) {
    TimeStuClass TempRTCData = {0};
    // Calculate seconds, minutes, hours, day, month, year from timestamp
    uint32_t remaining_seconds = timestamp;
    // Calculate seconds
    TempRTCData.second = remaining_seconds % 60;
    remaining_seconds /= 60;
    // Calculate minutes
    TempRTCData.minute = remaining_seconds % 60;
    remaining_seconds /= 60;
    // Calculate hours
    TempRTCData.hour = remaining_seconds % 24;
    remaining_seconds /= 24;
    // Calculate day, month, year
    uint32_t days = remaining_seconds;
    uint32_t year = 1970;
    while (1) {
        uint32_t days_in_year = isLeapYear(year) ? 366 : 365;
        if (days < days_in_year)
            break;
        days -= days_in_year;
        year++;
    }

    TempRTCData.year = year;
    uint8_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t month = 0;
    if (isLeapYear(year)) {
        days_in_month[1] = 29;
    }
    while (days >= days_in_month[month]) {
        days -= days_in_month[month];
        month++;
    }
    days_in_month[1] = 28; // Reset February days
    TempRTCData.month = month + 1;
    TempRTCData.day = days + 1;
    return TempRTCData;
}
// get week number
int getDayOfWeek(uint32_t iYear, uint32_t iMonth, uint32_t iDay) {
    int iWeek = 0;
    unsigned int y = 0, c = 0, m = 0, d = 0;

    iYear = anyBaseToAnyBase(iYear, 10, 16);
    iMonth = anyBaseToAnyBase(iMonth, 10, 16);
    iDay = anyBaseToAnyBase(iDay, 10, 16);

    if (iMonth == 1 || iMonth == 2) {
        c = (iYear - 1) / 100;
        y = (iYear - 1) % 100;
        m = iMonth + 12;
        d = iDay;
    } else {
        c = iYear / 100;
        y = iYear % 100;
        m = iMonth;
        d = iDay;
    }
    //蔡勒公式
    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
    // iWeek为负时取模
    iWeek = iWeek >= 0 ? (iWeek % 7) : (iWeek % 7 + 7);
    return iWeek;
}
