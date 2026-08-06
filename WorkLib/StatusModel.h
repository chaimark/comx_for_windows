#ifndef __STATUS_MODE_H__
#define __STATUS_MODE_H__

#include "StrLib.h"
#include <stdbool.h>
#include <stdint.h>

// 状态机任务函数表
typedef struct _StatusTaskFunc {
    uint16_t Id;
    void (*TaskFunc)(void *arg);
    void *arg;
    struct _StatusTaskFunc *next;
    struct _StatusTaskFunc *prev;
}StatusTaskFunc;
// 状态机
typedef struct _StatusDev { 
    uint16_t NumberOfStatus;
    uint16_t NextId;
    StatusTaskFunc *Head;
    int (*addTaskFuncNode)(struct _StatusDev This, uint16_t InputId, void (*TaskFunc)(void *), void *arg);
    void (*delTaskFuncById)(struct _StatusDev This, uint16_t InputId);
    void (*closeTaskFuncAll)(struct _StatusDev This);
    void (*ExecuteSteep)(struct _StatusDev This);
} StatusDev;

// 状态机初始化
extern StatusDev initStatusDev(uint16_t NumberOfStatus);

#endif
