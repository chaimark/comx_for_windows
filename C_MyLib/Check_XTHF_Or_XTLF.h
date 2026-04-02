#ifndef CHECK_XTHF_OR_XTLF_H
#define CHECK_XTHF_OR_XTLF_H

#ifdef OPEN_FL33LXX_LIB

// 0: 时钟正常 1: 低时钟异常 2: 高时钟异常
#define XT_CLOCK_OK 0
#define XTLF_FAIL 1
#define XTHF_FAIL 2

#include "StrLib.h"
#include <stdbool.h>
#include <stdint.h>

// 类定义
typedef struct _Check_XTHF_Or_XTLF {
    uint8_t is_XTClock_Error;
    void (*UserTask)(void);
} Check_XTHF_Or_XTLF;
extern Check_XTHF_Or_XTLF XTClock_CheckTask;

// 外部接口
extern void Config_Init_XTHF_And_XTLF(void);

#endif
#endif
