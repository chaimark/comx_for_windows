#ifndef __RTC_SETTIME_H__
#define __RTC_SETTIME_H__

#include "MIN_SetTime.h"
#include "PublicLib_No_One.h"
#include "StrLib.h"
#include <stdbool.h>
#include <stdint.h>

// 将小时转换为分钟
#define HourToMin(x) (x * 60)
// 将分钟转换为秒
#define MinToSec(x) (x * 60)

#ifdef OPEN_LOWPWER_DEBUG
typedef enum _RTCTaskName {
    LowPwerDebug,
    IWDTClS,
} RTCTaskName;
#else
typedef enum _RTCTaskName {
    IWDTClS,
    HomePageRefresh, // 用于判断什么时候开启主页面刷新功能
} RTCTaskName;
#endif

// 任务数量
#define _RTCTimeTaskMAX 2

#ifdef OPEN_LOWPWER_DEBUG
#define RTCTimeTaskMAX (_RTCTimeTaskMAX + 1)
#else
#define RTCTimeTaskMAX _RTCTimeTaskMAX
#endif

typedef struct _USER_SET_TASK {
    struct {
        bool TimeTask_Falge; // 当前计时任务是否完成
        bool isTaskStart;    // 当前计时任务是否开启
        uint64_t CountNum;
        uint64_t CountMaxNum;
        void (*TaskFunc)(void);
    } Task[RTCTimeTaskMAX];
    void (*InitSetTimeTask)(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void));
    void (*CloseTask)(int TaskAddr);
    int NumberOfTimeTask;
} USER_SET_TASK;
extern USER_SET_TASK RTC_TASK;
extern void SecondCountRTCTask(void);

#endif
