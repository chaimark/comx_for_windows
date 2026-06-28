#ifndef __PUBLIC_LIB_NO_ONE_H__
#define __PUBLIC_LIB_NO_ONE_H__

// 公共库头文件，用于包含一些非库使用的公共定义，或声明一些公共使用的全局结构体
#include "SetTime.h"
extern timetask RTC_Task;
extern timetask Uart_Task;
typedef struct _TimeStuClass {
    uint32_t year;   /** 年 */
    uint32_t month;  /** 月 */
    uint32_t day;    /** 日 */
    uint32_t week;   /** 周 */
    uint32_t hour;   /** 时 */
    uint32_t minute; /** 分 */
    uint32_t second; /** 秒 */
} TimeStuClass;

/*
 * 此为公共头文件不能在此文件包含其他有类型定义的头文件，
 * 上面是 WorkLib 库用到的相关的定义
 * 下面是用户程序相关的定义
 */

#if defined(USE_HAL_DRIVER)
#include "stm32f1xx_hal.h"
#endif
#define LIST_MAX_NUM 4
typedef enum {
    CmdNumber = 0,
    CmdBool,
    CmdString,
    CmdFloat,
} SysKeyType;
typedef struct _CmdNameList {
    char *Name;
    SysKeyType Type;
} CmdNameList_T;
extern CmdNameList_T CmdNameList[LIST_MAX_NUM];
//************************************************************************// 字段表定义
typedef struct _TestGPIO {
    bool TempStatus;
    uint8_t ReadBCD_Var;
    TimeStuClass TimeData;
} TestGPIO;
extern TestGPIO SystemRunData;
extern void Error_Handler(void);
//************************************************************************// 系统运行时数据
#define SYSTEM_START_SIGN 0x5A
typedef struct _ManagerEEprom {
    uint8_t RunSign;
} ManagerEEprom;
extern ManagerEEprom SystemManager;
//************************************************************************// 系统管理参数
#define ID_OF_CTRL_SUSPEND_DEFINED
typedef enum {
    UsDelayFun = 0,
    iicReadFun,
    iicWriteFun,
} IDOfCtrlSuspend;
//************************************************************************// 可以关闭调度器的函数ID
#endif
