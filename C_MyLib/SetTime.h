#ifndef __SETTIME_H__
#define __SETTIME_H__

#include "StrLib.h"
#include <stdbool.h>
#include <stdint.h>

#define SecTo250Ms(sec) (uint64_t)((sec * 4 == 0) ? 1 : (sec * 4))
#define SecTo10Ms(sec) (uint64_t)((sec * 100 == 0) ? 1 : (sec * 100))
#define SecTo5Ms(sec) (uint64_t)((sec * 200 == 0) ? 1 : (sec * 200))

/******************XXTIM********************/
typedef struct _Task_T {
    strnew Name;
    bool TimeTask_Falge; // 当前计时任务是否完成
    bool isTaskStart;    // 当前计时任务是否开启
    uint64_t CountNum;
    uint64_t CountMaxNum;
    void *arg;
    void (*TaskFunc)(void *);
    struct _Task_T *next;
    struct _Task_T *prev;
} Task_T;
typedef struct _timetask {
    int NumberOfTimeTask;
    Task_T *Head;
    Task_T *(*getTaskByName)(struct _timetask This, strnew Name);
    int (*addTaskNode)(struct _timetask This, strnew Name);
    void (*initTaskByName)(struct _timetask This, strnew Name, uint64_t CountMaxNum, void (*TaskFunc)(void *), void *arg);
    void (*stopTaskByName)(struct _timetask This, strnew Name);
    void (*closeTaskAll)(struct _timetask This);
} timetask;

// 定时任务初始化
extern timetask initSetTime(void);
// 定时器中断轮询 api
extern void CountSetTimeTask(timetask This);

#endif
