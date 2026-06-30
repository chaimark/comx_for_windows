#ifndef __TIMELIB__
#define __TIMELIB__

#include "PublicLib_No_One.h"
#include "NumberBaseLib.h"
#include "StrLib.h"
#include <stdint.h>
#ifdef __linux__
#include <unistd.h>
#include <time.h>
#elif defined(USE_HAL_DRIVER)
#else
#include <windows.h>
#endif
extern int isLeapYear(uint32_t year);
extern uint32_t get_timestamp(uint32_t NowYear, uint32_t NowMonth, uint32_t NowDay, uint32_t NowHour, uint32_t NowMinute, uint32_t NowSecond);
extern uint32_t getTimeNumber_UTCByRTCTime(strnew RTCTime_String);
extern TimeStuClass timestampToRTCData(uint32_t timestamp);
extern int getDayOfWeek(uint32_t iYear, uint32_t iMonth, uint32_t iDay);
#ifdef USE_RTOS
#if (USE_RTOS == 0U) // 使用 RTOS
#include "FreeRTOS.h"
#include "task.h"
#ifndef ID_OF_CTRL_SUSPEND_DEFINED
#define ID_OF_CTRL_SUSPEND_DEFINED
typedef enum {
    UsDelayFun = 0xF0,
} IDOfCtrlSuspend;
#endif
extern void closeOrOpenTaskSuspendAll(IDOfCtrlSuspend CtrID, bool IsPause);
#endif
#endif
extern void DelayUs_General(uint32_t Delay);
static inline void DelayMs_General(uint32_t Delay) {
#if defined(USE_HAL_DRIVER)
    HAL_Delay(Delay);
#else
    (void)Delay;
#endif
}

#endif
