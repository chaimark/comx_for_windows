#include "RTC_SetTime.h"

static void _RTCCloseTask(int TaskAddr);
static void _InitRTCTask(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void));

USER_SET_TASK RTC_TASK = {
    .Task = {{0}},
    .InitSetTimeTask = _InitRTCTask,
    .CloseTask = _RTCCloseTask,
    .NumberOfTimeTask = RTCTimeTaskMAX, // 定时任务数量
};

static void _RTCCloseTask(int TaskAddr) {
    if ((TaskAddr < 0) || (TaskAddr >= RTCTimeTaskMAX)) {
        return;
    }
    RTC_TASK.Task[TaskAddr].isTaskStart = false; // 初始化标记
    RTC_TASK.Task[TaskAddr].TimeTask_Falge = false;
    RTC_TASK.Task[TaskAddr].CountNum = 0; // 复位初始
    RTC_TASK.Task[TaskAddr].TaskFunc = NULL;
}
static void _InitRTCTask(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void)) {
    if ((TaskAddr < 0) || (TaskAddr >= RTCTimeTaskMAX)) {
        return;
    }
    RTC_TASK.Task[TaskAddr].TimeTask_Falge = (SetCountMaxNum == 0 ? true : false); // 初始化标记
    RTC_TASK.Task[TaskAddr].isTaskStart = true;                                    // 开启
    RTC_TASK.Task[TaskAddr].CountMaxNum = SetCountMaxNum;                          // 定时任务点
    RTC_TASK.Task[TaskAddr].CountNum = 0;                                          // 复位初始
    RTC_TASK.Task[TaskAddr].TaskFunc = TaskFunc;
    if (RTC_TASK.Task[TaskAddr].TimeTask_Falge == true) {
        if (RTC_TASK.Task[TaskAddr].TaskFunc != NULL) {
            RTC_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
        }
    }
}
// 计数函数
void SecondCountRTCTask(void) {
    /*****************************************************/
    if (!RTC_TASK.Task[IWDTClS].TimeTask_Falge) { // 8 分钟后没有返回主函数重新复位计数器, 复位MCU
        // FL_IWDT_ReloadCounter(IWDT);    // 8 分钟之内 RTC 帮忙喂狗
    }
    /*****************************************************/
    for (int TaskAddr = 0; TaskAddr < RTCTimeTaskMAX; TaskAddr++) {
        if (RTC_TASK.Task[TaskAddr].isTaskStart == false) {
            continue;
        }
        if (RTC_TASK.Task[TaskAddr].CountNum < RTC_TASK.Task[TaskAddr].CountMaxNum) {
            RTC_TASK.Task[TaskAddr].CountNum++;
        }
        if (RTC_TASK.Task[TaskAddr].CountNum >= RTC_TASK.Task[TaskAddr].CountMaxNum) {
            RTC_TASK.Task[TaskAddr].TimeTask_Falge = true;
        } else {
            RTC_TASK.Task[TaskAddr].TimeTask_Falge = false;
        }
        if (RTC_TASK.Task[TaskAddr].TimeTask_Falge == true) {
            if (RTC_TASK.Task[TaskAddr].TaskFunc != NULL) {
                RTC_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
            }
        }
    }
#ifdef TimeSpeedNum
    TimeSpeed();
#endif
}
