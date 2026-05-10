#ifndef __SETTIME_H__
#define __SETTIME_H__

#include "StrLib.h"
#include <stdbool.h>
#include <stdint.h>

#define SecTo250Ms(sec) (uint64_t)((sec * 4 == 0) ? 1 : (sec * 4))
#define SecTo10Ms(sec) (uint64_t)((sec * 100 == 0) ? 1 : (sec * 100))
#define SecTo5Ms(sec) (uint64_t)((sec * 200 == 0) ? 1 : (sec * 200))

#define BSTIM_OPEN_AND_TASK_NUM 2
// #define LPTIM_OPEN_AND_TASK_NUM 0

#ifdef BSTIM_OPEN_AND_TASK_NUM
/********************BSTIM********************/
typedef enum _TimeTaskName { // 发送AT指令时调用什么函数发送, 对应的函数名
    RecTimeLPUART0,          // 用于判断 LPuart0 串口是否接收结束
    RecTimeLPUART1,          // 用于判断 LPuart1 串口是否接收结束
} TimeTaskName;

typedef struct _BSTIM_USER_SET_TASK {
    struct {
        bool TimeTask_Falge; // 当前计时任务是否完成
        bool isTaskStart;    // 当前计时任务是否开启
        uint64_t CountNum;
        uint64_t CountMaxNum;
        void (*TaskFunc)(void);
    } Task[BSTIM_OPEN_AND_TASK_NUM];
    void (*InitSetTimeTask)(int TaskAddr, uint64_t SetMax10MsNum, void (*TaskFunc)(void));
    void (*CloseTask)(int TaskAddr);
    int NumberOfTimeTask;
} BSTIM_USER_SET_TASK;
extern BSTIM_USER_SET_TASK SetTime;
extern void CountSetTimeTask(void);
#endif

#ifdef LPTIM_OPEN_AND_TASK_NUM
/********************LPTIM********************/
typedef enum _LPTimeTaskName {

} LPTimeTaskName;

#define SecTo250Ms(sec) (uint64_t)((sec * 4 == 0) ? 1 : (sec * 4))
typedef struct _LPTIM_USER_SET_TASK {
    struct {
        bool TimeTask_Falge; // 当前计时任务是否完成
        bool isTaskStart;    // 当前计时任务是否开启
        uint64_t CountNum;
        uint64_t CountMaxNum;
        void (*TaskFunc)(void);
    } Task[LPTIM_OPEN_AND_TASK_NUM];
    void (*LPInitSetTimeTask)(int TaskAddr, uint64_t SetMax250MsNum, void (*TaskFunc)(void));
    void (*CloseTask)(int TaskAddr);
    int NumberOfTimeTask;
} LPTIM_USER_SET_TASK;
extern LPTIM_USER_SET_TASK SetLPTime;
extern void LPCountSetTimeTask(void);
#endif
#endif
