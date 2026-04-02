#include "SetTime.h"
#ifdef BSTIM_OPEN_AND_TASK_NUM

static void _SetCloseTask(int TaskAddr);
static void _InitSetTimeTask(int TaskAddr, uint64_t SetMax10MsNum, void (*TaskFunc)(void));

// 可自定义的定时任务
BSTIM_USER_SET_TASK SetTime = {
    .Task = {{0}},
    .InitSetTimeTask = _InitSetTimeTask,
    .CloseTask = _SetCloseTask,
    .NumberOfTimeTask = BSTIM_OPEN_AND_TASK_NUM, // 定时任务数量
};

static void _SetCloseTask(int TaskAddr) {
    if ((TaskAddr < 0) || (TaskAddr >= BSTIM_OPEN_AND_TASK_NUM)) {
        return;
    }
    SetTime.Task[TaskAddr].isTaskStart = false; // 初始化标记
    SetTime.Task[TaskAddr].TimeTask_Falge = false;
    SetTime.Task[TaskAddr].CountNum = 0; // 复位初始
    SetTime.Task[TaskAddr].TaskFunc = NULL;
}
static void _InitSetTimeTask(int TaskAddr, uint64_t SetMax10MsNum, void (*TaskFunc)(void)) {
    if ((TaskAddr < 0) || (TaskAddr >= BSTIM_OPEN_AND_TASK_NUM)) {
        return;
    }
    SetTime.Task[TaskAddr].TimeTask_Falge = (SetMax10MsNum == 0 ? true : false); // 初始化标记
    SetTime.Task[TaskAddr].isTaskStart = true;                                   // 开启
    SetTime.Task[TaskAddr].CountMaxNum = SetMax10MsNum;                          // 定时任务点
    SetTime.Task[TaskAddr].CountNum = 0;                                         // 复位初始
    SetTime.Task[TaskAddr].TaskFunc = TaskFunc;
    if (SetTime.Task[TaskAddr].TimeTask_Falge == true) {
        if (SetTime.Task[TaskAddr].TaskFunc != NULL) {
            SetTime.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
        }
    }
}
// 计数函数
void CountSetTimeTask(void) {
    for (int TaskAddr = 0; TaskAddr < BSTIM_OPEN_AND_TASK_NUM; TaskAddr++) {
        if (SetTime.Task[TaskAddr].isTaskStart == false) {
            continue;
        }
        if (SetTime.Task[TaskAddr].CountNum < SetTime.Task[TaskAddr].CountMaxNum) {
            SetTime.Task[TaskAddr].CountNum++;
        }
        if (SetTime.Task[TaskAddr].CountNum >= SetTime.Task[TaskAddr].CountMaxNum) {
            SetTime.Task[TaskAddr].TimeTask_Falge = true;
        } else {
            SetTime.Task[TaskAddr].TimeTask_Falge = false;
        }
        if (SetTime.Task[TaskAddr].TimeTask_Falge == true) {
            if (SetTime.Task[TaskAddr].TaskFunc != NULL) {
                SetTime.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
            }
        }
    }
}
#endif

////////////////////////////////////////////////////
#ifdef LPTIM_OPEN_AND_TASK_NUM

static void _LPSetCloseTask(int TaskAddr);
static void _LPInitSetTimeTask(int TaskAddr, uint64_t SetMax250MsNum, void (*TaskFunc)(void));

// 可自定义的定时任务
LPTIM_USER_SET_TASK SetLPTime = {
    .Task = {{0}},
    .LPInitSetTimeTask = _LPInitSetTimeTask,
    .CloseTask = _LPSetCloseTask,
    .NumberOfTimeTask = LPTIM_OPEN_AND_TASK_NUM, // 定时任务数量
};

static void _LPSetCloseTask(int TaskAddr) {
    if ((TaskAddr < 0) || (TaskAddr >= LPTIM_OPEN_AND_TASK_NUM)) {
        return;
    }
    SetLPTime.Task[TaskAddr].isTaskStart = false; // 初始化标记
    SetLPTime.Task[TaskAddr].TimeTask_Falge = false;
    SetLPTime.Task[TaskAddr].CountNum = 0; // 复位初始
    SetLPTime.Task[TaskAddr].TaskFunc = NULL;
}
static void _LPInitSetTimeTask(int TaskAddr, uint64_t SetMax250MsNum, void (*TaskFunc)(void)) {
    if ((TaskAddr < 0) || (TaskAddr >= LPTIM_OPEN_AND_TASK_NUM)) {
        return;
    }
    SetLPTime.Task[TaskAddr].TimeTask_Falge = (SetMax250MsNum == 0 ? true : false); // 初始化标记
    SetLPTime.Task[TaskAddr].isTaskStart = true;                                    // 开启
    SetLPTime.Task[TaskAddr].CountMaxNum = SetMax250MsNum;                          // 定时任务点
    SetLPTime.Task[TaskAddr].CountNum = 0;                                          // 复位初始
    SetLPTime.Task[TaskAddr].TaskFunc = TaskFunc;
    if (SetLPTime.Task[TaskAddr].TimeTask_Falge == true) {
        if (SetLPTime.Task[TaskAddr].TaskFunc != NULL) {
            SetLPTime.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
        }
    }
}
#include "mf_config.h"
// 计数函数
void LPCountSetTimeTask(void) {
    for (int TaskAddr = 0; TaskAddr < LPTIM_OPEN_AND_TASK_NUM; TaskAddr++) {
        if (SetLPTime.Task[TaskAddr].isTaskStart == false) {
            continue;
        }
        if (SetLPTime.Task[TaskAddr].CountNum < SetLPTime.Task[TaskAddr].CountMaxNum) {
            SetLPTime.Task[TaskAddr].CountNum++;
        }
        if (SetLPTime.Task[TaskAddr].CountNum >= SetLPTime.Task[TaskAddr].CountMaxNum) {
            SetLPTime.Task[TaskAddr].TimeTask_Falge = true;
        } else {
            SetLPTime.Task[TaskAddr].TimeTask_Falge = false;
        }
        if (SetLPTime.Task[TaskAddr].TimeTask_Falge == true) {
            if (SetLPTime.Task[TaskAddr].TaskFunc != NULL) {
                SetLPTime.Task[TaskAddr].TaskFunc(); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
            }
        }
    }
}
#endif
