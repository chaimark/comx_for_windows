#include "TimeLib.h"
#include <stdio.h>

// 判断是否为闰年
int isLeapYear(uint32_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
// 计算从1970年1月1日到指定日期的总天数
static int calculate_days(int year, int month, int day) {
    int DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int TotalDays = 0;
    int i;

    // 计算从1970年到当前年份的总天数
    for (i = 1970; i < year; i++) {
        TotalDays += (isLeapYear(i) ? 366 : 365);
    }

    // 计算当前年份从1月1日到指定日期的天数
    for (i = 0; i < month - 1; i++) {
        TotalDays += DaysInMonth[i];
        if (i == 1 && isLeapYear(year)) { // 2月需要额外加一天(闰年)
            TotalDays++;
        }
    }
    // 加上当前月份的天数
    TotalDays += day - 1;
    return TotalDays;
}
// 计算时间戳(秒数)
uint32_t get_timestamp(uint32_t NowYear, uint32_t NowMonth, uint32_t NowDay, uint32_t NowHour, uint32_t NowMinute,
                       uint32_t NowSecond) {
    // 计算从1970年1月1日到指定日期的总天数
    int Days = calculate_days(NowYear, NowMonth, NowDay);
    // 转换为秒数
    long OverTimeSec = Days * 86400LL; // 每天86400秒
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
    static const int DaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint32_t Days = 0;

    // 计算从1970到当前年份的天数
    for (int y = 1970; y < year; y++) {
        Days += 365;
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
            Days += 1; // 闰年
    }

    // 计算当前年内已过的月份天数
    for (int m = 1; m < month; m++) {
        Days += DaysInMonth[m - 1];
        if (m == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
            Days += 1; // 当前年闰年2月
    }

    Days += (day - 1);

    uint32_t Timestamp = Days * 86400 + hour * 3600 + min * 60 + sec;
    return Timestamp;
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
        uint32_t DaysInYear = isLeapYear(year) ? 366 : 365;
        if (days < DaysInYear)
            break;
        days -= DaysInYear;
        year++;
    }

    TempRTCData.year = year;
    uint8_t DaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t month = 0;
    if (isLeapYear(year)) {
        DaysInMonth[1] = 29;
    }
    while (days >= DaysInMonth[month]) {
        days -= DaysInMonth[month];
        month++;
    }
    DaysInMonth[1] = 28; // Reset February days
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

#if (USE_RTOS == 1U)
void RTOS_DelayUs(uint32_t nus) {
    uint32_t ticks;
    uint32_t told, tnow, reload, tcnt = 0;
    reload = SysTick->LOAD;                    //获取重装载寄存器值
    ticks = nus * (SystemCoreClock / 1000000); //计数时间值 括号里的代表1us秒嘀嗒定时器的value会向下降多少值
    vTaskSuspendAll();                         // 阻止OS调度，防止打断us延时
    told = SysTick->VAL;                       //获取当前数值寄存器值（开始时数值）
    while (1) {
        tnow = SysTick->VAL; //获取当前数值寄存器值
        if (tnow != told)    //当前值不等于开始值说明已在计数
        {
            if (tnow < told)              //当前值小于开始数值，说明未计到0
                tcnt += told - tnow;      //计数值=开始值-当前值else //当前值大于开始数值，说明已计到0并重新计数
            else
                tcnt += reload - tnow + told; //计数值=重装载值-当前值+开始值 （已//从开始值计到0）
            told = tnow;                  //更新开始值
            if (tcnt >= ticks)
                break; //时间超过/等于要延迟的时间,则退出.
        }
    }
    xTaskResumeAll(); // 恢复OS调度
}
#endif
