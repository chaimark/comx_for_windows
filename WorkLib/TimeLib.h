#ifndef __TIME_LIB_H__
#define __TIME_LIB_H__

// #define RTTHREAD_OS   /* 嵌入式实时系统 */
// #define FREERTOS_OS   /* 嵌入式实时系统 */
// #define ON_ONE_OS     /* 裸机系统 */

/* 是否使用 IDOfCtrlSuspend 定义的函数ID */
#define ID_OF_CTRL_SUSPEND_DEFINED
/* 没有使用时 HAL 库 需要补充 实现 HAL_Delay 函数 */
// #define USER_Delay_General 

#include "NumberBaseLib.h"
#include "StrLib.h"
#include <stdint.h>
typedef struct _TimeStuClass {
    uint32_t year;   /** 年 */
    uint32_t month;  /** 月 */
    uint32_t day;    /** 日 */
    uint32_t week;   /** 周 */
    uint32_t hour;   /** 时 */
    uint32_t minute; /** 分 */
    uint32_t second; /** 秒 */
} TimeStuClass;

#ifdef __linux__
#include <time.h>
#include <unistd.h>
#elif defined(USER_Delay_General)
#else
#include <windows.h>
#endif
extern int isLeapYear(uint32_t year);
extern uint32_t get_timestamp(uint32_t NowYear, uint32_t NowMonth, uint32_t NowDay, uint32_t NowHour, uint32_t NowMinute, uint32_t NowSecond);
extern uint32_t getTimeNumber_UTCByRTCTime(strnew RTCTime_String);
extern TimeStuClass timestampToRTCData(uint32_t timestamp);
extern int getDayOfWeek(uint32_t iYear, uint32_t iMonth, uint32_t iDay);
#if defined(FREERTOS_OS) || defined(RTTHREAD_OS) || defined(ON_ONE_OS)
#ifdef FREERTOS_OS
#include "FreeRTOS.h"
#include "task.h"
#endif
#ifdef RTTHREAD_OS
#define vTaskSuspendAll() rt_enter_critical() // 暂停调度器
#define xTaskResumeAll()  rt_exit_critical()  // 恢复调度器
#include <rtthread.h>
#endif
#ifndef ID_OF_CTRL_SUSPEND_DEFINED
#define ID_OF_CTRL_SUSPEND_DEFINED
typedef enum {
    UsDelayFun = 0,
} IDOfCtrlSuspend;
#else
#include "PublicLib_No_One.h"
#endif
#if defined(FREERTOS_OS) || defined(RTTHREAD_OS)
extern void closeOrOpenTaskSuspendAll(IDOfCtrlSuspend CtrID, bool IsPause);
#endif
#endif
extern void DelayUs_General(uint32_t Delay);
static inline void DelayMs_General(uint32_t Delay) {
#if defined(USER_Delay_General)
    extern void HAL_Delay(uint32_t Delay);
    HAL_Delay(Delay);
#else
    (void)Delay;
#endif
}

#endif
