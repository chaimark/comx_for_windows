#ifndef __PUBLIC_LIB_NO_ONE_H__
#define __PUBLIC_LIB_NO_ONE_H__

// 公共库头文件, 用于包含一些非库使用的公共定义, 或声明一些公共使用的全局结构体
// #define USER_Delay_General /* 没有使用时 HAL 库 需要补充 实现 HAL_Delay 函数 */
#include "SetTime.h"
extern timetask RTC_Task;
extern timetask Uart_Task;
extern timetask Tim_Task;
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
 * 此为公共头文件不能在此文件包含其他有类型定义的头文件, * 上面是 WorkLib 库用到的相关的定义
 * 下面是用户程序相关的定义
 */

// NOW_SYSTEM_VERSION 的长度 Len = 20-3-1 = 16
#define NOW_SYSTEM_VERSION "v0.16"
#define NOW_EEPROM_VERSION 13

/*
    malloc 一个 timetask  的总字节大小 = 36 + TaskNum * 72
    malloc 一个 StatusDev 的总字节大小 = 28 + DevNum * 20
    Heap_Size >= ((3 * 36 + 12 * 73) + (28 * 2 + 7 * 20)) * 1.5 = 1180 * 1.5 = 1770

    一共 12 个中断, 每个 4 个字节, 所以中断栈大小为 48 字节, 最大深度约7次
    Stack_Size >= 12 * 4 * 7 * 1.5 = 504 字节
*/

/*
    硬件中断优先级设为 0~4：属于紧急中断, 响应最快, 但不能调用 RTOS API。
    硬件中断优先级设为 6~15：属于普通中断, 可以安全调用 FromISR 函数。
    软件定时器优先级 configTIMER_TASK_PRIORITY 32 高于普通线程
    rtos 临界优先级 configMAX_SYSCALL_INTERRUPT_PRIORITY
    线程正常优先级 osPriorityNormal
*/
#endif
