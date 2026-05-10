#include "MIN_SetTime.h"

#ifdef _MINTimeTaskMAX
static void _MinRTCCloseTask(int TaskAddr);
static void _MinInitRTCTask(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void));

USER_MIN_SET_TASK MIN_TASK = {
    .Task = {{0}},
    .InitSetTimeTask = _MinInitRTCTask,
    .CloseTask = _MinRTCCloseTask,
    .NumberOfTimeTask = MINTimeTaskMAX, // 定时任务数量
};

static void _MinRTCCloseTask(int TaskAddr) {
    if ((TaskAddr < 0) || (TaskAddr >= MINTimeTaskMAX)) {
        return;
    }
    MIN_TASK.Task[TaskAddr].isTaskStart = false; // 初始化标记
    MIN_TASK.Task[TaskAddr].TimeTask_Falge = false;
    MIN_TASK.Task[TaskAddr].CountNum = 0; // 复位初始
    MIN_TASK.Task[TaskAddr].TaskFunc = NULL;
}
static void _MinInitRTCTask(int TaskAddr, uint64_t SetMaxCountNum, void (*TaskFunc)(void)) {
    if ((TaskAddr < 0) || (TaskAddr >= MINTimeTaskMAX)) {
        return;
    }
    MIN_TASK.Task[TaskAddr].TimeTask_Falge = (SetMaxCountNum == 0 ? true : false); // 初始化标记
    MIN_TASK.Task[TaskAddr].isTaskStart = true;                                    // 开启
    MIN_TASK.Task[TaskAddr].CountMaxNum = SetMaxCountNum;                          // 定时任务点
    MIN_TASK.Task[TaskAddr].CountNum = 0;                                          // 复位初始
    MIN_TASK.Task[TaskAddr].TaskFunc = TaskFunc;
    if (MIN_TASK.Task[TaskAddr].TimeTask_Falge == true) {
        if (MIN_TASK.Task[TaskAddr].TaskFunc != NULL) {
            MIN_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
        }
    }
}
// 计数函数
void MinCountRTCTask(void) {
    for (int TaskAddr = 0; TaskAddr < MINTimeTaskMAX; TaskAddr++) {
        if (MIN_TASK.Task[TaskAddr].isTaskStart == false) {
            continue;
        }
        if (MIN_TASK.Task[TaskAddr].CountNum < MIN_TASK.Task[TaskAddr].CountMaxNum) {
            MIN_TASK.Task[TaskAddr].CountNum++;
        }
        if (MIN_TASK.Task[TaskAddr].CountNum >= MIN_TASK.Task[TaskAddr].CountMaxNum) {
            MIN_TASK.Task[TaskAddr].TimeTask_Falge = true;
        } else {
            MIN_TASK.Task[TaskAddr].TimeTask_Falge = false;
        }
        if (MIN_TASK.Task[TaskAddr].TimeTask_Falge == true) {
            if (MIN_TASK.Task[TaskAddr].TaskFunc != NULL) {
                MIN_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
            }
        }
    }
}
#endif
//////////////////////////////////////////////////////////////
#ifdef Min_10_TimeTaskMAX
static void _Min_10_RTCCloseTask(int TaskAddr);
static void _Min_10_InitRTCTask(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void));

USER_Min_10_SET_TASK Min_10_TASK = {
    .Task = {{0}},
    .InitSetTimeTask = _Min_10_InitRTCTask,
    .CloseTask = _Min_10_RTCCloseTask,
    .NumberOfTimeTask = Min_10_TimeTaskMAX, // 定时任务数量
};

static void _Min_10_RTCCloseTask(int TaskAddr) {
    if ((TaskAddr < 0) || (TaskAddr >= Min_10_TimeTaskMAX)) {
        return;
    }
    Min_10_TASK.Task[TaskAddr].isTaskStart = false; // 初始化标记
    Min_10_TASK.Task[TaskAddr].TimeTask_Falge = false;
    Min_10_TASK.Task[TaskAddr].CountNum = 0; // 复位初始
    Min_10_TASK.Task[TaskAddr].TaskFunc = NULL;
}
static void _Min_10_InitRTCTask(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void)) {
    if ((TaskAddr < 0) || (TaskAddr >= Min_10_TimeTaskMAX)) {
        return;
    }
    Min_10_TASK.Task[TaskAddr].TimeTask_Falge = (SetCountMaxNum == 0 ? true : false); // 初始化标记
    Min_10_TASK.Task[TaskAddr].isTaskStart = true;                                    // 开启
    Min_10_TASK.Task[TaskAddr].CountMaxNum = SetCountMaxNum;                          // 定时任务点
    Min_10_TASK.Task[TaskAddr].CountNum = 0;                                          // 复位初始
    Min_10_TASK.Task[TaskAddr].TaskFunc = TaskFunc;
    if (Min_10_TASK.Task[TaskAddr].TimeTask_Falge == true) {
        if (Min_10_TASK.Task[TaskAddr].TaskFunc != NULL) {
            Min_10_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
        }
    }
}
// 计数函数
void Min_10_CountRTCTask(void) {
    for (int TaskAddr = 0; TaskAddr < Min_10_TimeTaskMAX; TaskAddr++) {
        if (Min_10_TASK.Task[TaskAddr].isTaskStart == false) {
            continue;
        }
        if (Min_10_TASK.Task[TaskAddr].CountNum < Min_10_TASK.Task[TaskAddr].CountMaxNum) {
            Min_10_TASK.Task[TaskAddr].CountNum++;
        }
        if (Min_10_TASK.Task[TaskAddr].CountNum >= Min_10_TASK.Task[TaskAddr].CountMaxNum) {
            Min_10_TASK.Task[TaskAddr].TimeTask_Falge = true;
        } else {
            Min_10_TASK.Task[TaskAddr].TimeTask_Falge = false;
        }
        if (Min_10_TASK.Task[TaskAddr].TimeTask_Falge == true) {
            if (Min_10_TASK.Task[TaskAddr].TaskFunc != NULL) {
                Min_10_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
            }
        }
    }
}
#endif
//////////////////////////////////////////////////////////////
#ifdef Min_15_TimeTaskMAX
static void _Min_15_RTCCloseTask(int TaskAddr);
static void _Min_15_InitRTCTask(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void));

USER_Min_15_SET_TASK Min_15_TASK = {
    .Task = {{0}},
    .InitSetTimeTask = _Min_15_InitRTCTask,
    .CloseTask = _Min_15_RTCCloseTask,
    .NumberOfTimeTask = Min_15_TimeTaskMAX, // 定时任务数量
};

static void _Min_15_RTCCloseTask(int TaskAddr) {
    if ((TaskAddr < 0) || (TaskAddr >= Min_15_TimeTaskMAX)) {
        return;
    }
    Min_15_TASK.Task[TaskAddr].isTaskStart = false; // 初始化标记
    Min_15_TASK.Task[TaskAddr].TimeTask_Falge = false;
    Min_15_TASK.Task[TaskAddr].CountNum = 0; // 复位初始
    Min_15_TASK.Task[TaskAddr].TaskFunc = NULL;
}
static void _Min_15_InitRTCTask(int TaskAddr, uint64_t SetCountMaxNum, void (*TaskFunc)(void)) {
    if ((TaskAddr < 0) || (TaskAddr >= Min_15_TimeTaskMAX)) {
        return;
    }
    Min_15_TASK.Task[TaskAddr].TimeTask_Falge = (SetCountMaxNum == 0 ? true : false); // 初始化标记
    Min_15_TASK.Task[TaskAddr].isTaskStart = true;                                    // 开启
    Min_15_TASK.Task[TaskAddr].CountMaxNum = SetCountMaxNum;                          // 定时任务点
    Min_15_TASK.Task[TaskAddr].CountNum = 0;                                          // 复位初始
    Min_15_TASK.Task[TaskAddr].TaskFunc = TaskFunc;
    if (Min_15_TASK.Task[TaskAddr].TimeTask_Falge == true) {
        if (Min_15_TASK.Task[TaskAddr].TaskFunc != NULL) {
            Min_15_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
        }
    }
}
// 计数函数
void Min_15_CountRTCTask(void) {
    for (int TaskAddr = 0; TaskAddr < Min_15_TimeTaskMAX; TaskAddr++) {
        if (Min_15_TASK.Task[TaskAddr].isTaskStart == false) {
            continue;
        }
        if (Min_15_TASK.Task[TaskAddr].CountNum < Min_15_TASK.Task[TaskAddr].CountMaxNum) {
            Min_15_TASK.Task[TaskAddr].CountNum++;
        }
        if (Min_15_TASK.Task[TaskAddr].CountNum >= Min_15_TASK.Task[TaskAddr].CountMaxNum) {
            Min_15_TASK.Task[TaskAddr].TimeTask_Falge = true;
        } else {
            Min_15_TASK.Task[TaskAddr].TimeTask_Falge = false;
        }
        if (Min_15_TASK.Task[TaskAddr].TimeTask_Falge == true) {
            if (Min_15_TASK.Task[TaskAddr].TaskFunc != NULL) {
                Min_15_TASK.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
            }
        }
    }
}
#endif
//////////////////////////////////////////////////////////////
