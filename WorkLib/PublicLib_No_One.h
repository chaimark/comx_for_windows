#ifndef __PUBLIC_LIB_NO_ONE_H__
#define __PUBLIC_LIB_NO_ONE_H__

// 公共库头文件，用于包含一些非库使用的公共定义，或声明一些公共使用的全局结构体
#include "SetTime.h"
extern timetask RTC_Task;
extern timetask Uart_Task;
typedef struct _TimeStuClass {
    uint32_t year;   /** 年 */
    uint32_t month;  /** 月 */
    uint32_t day;    /** 日 */
    uint32_t week;   /** 周 */
    uint32_t hour;   /** 时 */
    uint32_t minute; /** 分 */
    uint32_t second; /** 秒 */
} TimeStuClass;

/*
 * 此为公共头文件不能在此文件包含其他有类型定义的头文件，
 * 上面是 WorkLib 库用到的相关的定义
 * 下面是用户程序相关的定义
 */
#endif

