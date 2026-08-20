#ifndef __TIME_LIB_H__
#define __TIME_LIB_H__

#include "PublicLib_No_One.h"
#include "NumberBaseLib.h"
#include "StrLib.h"
#include <stdint.h>
#ifdef __linux__
#include <unistd.h>
#include <time.h>
#elif defined(__STM32F1xx_HAL_H)
#else
#include <windows.h>
#endif
extern int isLeapYear(uint32_t year);
extern uint32_t get_timestamp(uint32_t NowYear, uint32_t NowMonth, uint32_t NowDay, uint32_t NowHour, uint32_t NowMinute, uint32_t NowSecond);
extern uint32_t getTimeNumber_UTCByRTCTime(strnew RTCTime_String);
extern TimeStuClass timestampToRTCData(uint32_t timestamp);
extern int getDayOfWeek(uint32_t iYear, uint32_t iMonth, uint32_t iDay);
#if defined(FREERTOS_CONFIG_H) || defined(__RTTHREAD_CFG_H__)
#ifdef FREERTOS_CONFIG_H
#include "FreeRTOS.h"
#include "task.h"
#endif
#ifdef __RTTHREAD_CFG_H__
#define vTaskSuspendAll() rt_enter_critical() // 暂停调度器
#define xTaskResumeAll()  rt_exit_critical()  // 恢复调度器
#include <rtthread.h>
#endif
#ifndef ID_OF_CTRL_SUSPEND_DEFINED
#define ID_OF_CTRL_SUSPEND_DEFINED
typedef enum {
    UsDelayFun = 0xF0,
} IDOfCtrlSuspend;
#endif
extern void closeOrOpenTaskSuspendAll(IDOfCtrlSuspend CtrID, bool IsPause);
#endif
extern void DelayUs_General(uint32_t Delay);
static inline void DelayMs_General(uint32_t Delay) {
#if defined(__STM32F1xx_HAL_H)
    HAL_Delay(Delay);
#else
    (void)Delay;
#endif
}

#endif
