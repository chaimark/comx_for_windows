#ifndef __PUBLIC_LIB_NO_ONE_H__
#define __PUBLIC_LIB_NO_ONE_H__

// 公共库头文件, 用于包含一些非库使用的公共定义, 或声明一些公共使用的全局结构体
#include "FreeRTOSConfig.h"
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
 * 此为公共头文件不能在此文件包含其他有类型定义的头文件,
 * 上面是 WorkLib 库用到的相关的定义
 * 下面是用户程序相关的定义
 */

// NOW_SYSTEM_VERSION 的长度 Len = 20-3-1 = 16
#define NOW_SYSTEM_VERSION "v0.12"
#define NOW_EEPROM_VERSION 12
extern void Error_Handler(void);

#define OPEN_SPI

typedef enum {
    CmdNumber = 0,
    CmdBool,
    CmdString,
    CmdFloat,
    CmdObject,
} SysKeyType;

// 字段表
#define __ClearPwm__           "ClearPwm"
#define __ReBoot__             "ReBoot"
#define __LiftLock__           "LiftLock"
#define __GpioWay1__           "GpioWay1"
#define __GpioWay2__           "GpioWay2"
#define __GpioWay3__           "GpioWay3"
#define __GpioWay4__           "GpioWay4"
#define __StartCarLen__        "StartCarLen"
#define __ModBusSubAddr__      "ModBusSubAddr"
#define __NowResistanceFront__ "NowResistanceFront"
#define __NowResistanceBack__  "NowResistanceBack"
#define __BResistanceFront__   "BResistanceFront"
#define __BResistanceBack__    "BResistanceBack"
#define __KResistanceFront__   "KResistanceFront"
#define __KResistanceBack__    "KResistanceBack"
#define __CarLen__             "CarLen"
#define __DegreeSpeed__        "DegreeSpeed"
#define __KCarLen__            "KCarLen"
#define __KDegreeSpeed__       "KDegreeSpeed"
#define __PwmBasePoint__       "PwmBasePoint"
#define __SetDevId__           "SetDevId"
#define __VersionId__          "VersionId"
#define __PwmOutput__          "PwmOutput"
#define __UartManger__         "UartManger"
#define __Inverter__           "Inverter"
#define __PIDTestCar__         "PIDTestCar"

#define LIST_MAX_NUM 27
typedef struct _CmdNameList {
    char *Name;
    SysKeyType Type;
} CmdNameList_T;
extern CmdNameList_T CmdNameList[LIST_MAX_NUM];
////////////////***********************************//////////////// 字段表定义
typedef enum {
    MotorStateErr = -2,
    MotorStateReverse = -1,
    MotorStateStop = 0,
    MotorStateRun = 1,
} MotorState_T;
typedef struct _Inverter {
    bool MotorType;           // 电机类型
    uint16_t MaxOutFreqOfSet; // 变频器设定的最大输出频率
    uint16_t MotorFreqNow;    // 电机当前的频率
    MotorState_T MotorStatus; // 电机状态
} Inverter_T;
////////////////***********************************////////////////
///系统运行时内存中变频器参数
typedef struct _PIDTestCar {
    bool IsOpenPid;
    int32_t GoalResistanceFront;
    int32_t GoalResistanceBack;
} PIDTestCar_T;
////////////////***********************************//////////////// 系统运行时
/// PID 控制模式参数
typedef struct _SystemRun {
    TimeStuClass TimeData;
    char VersionId[20]; // 19 位版本号
    bool IsCheckSys;
    bool IsOkOfCar;
    bool LiftLock;
    PIDTestCar_T PIDTestCar;
    uint16_t GpioWayBit;
    uint32_t NowResistanceFront;
    uint32_t NowResistanceBack;
    float NowCarLen;
    float NowDegreeSpeed;
    Inverter_T Inverter;
    int Ch1FreqHz;
    int Ch2FreqHz;
    float Ch1HighPercent;
    float Ch2HighPercent;
#if (configCHECK_FOR_STACK_OVERFLOW == 1)
    unsigned long freeStack_CheckIwdg;
    unsigned long freeStack_EEpromTask;
    unsigned long freeStack_MeasureTask;
    unsigned long freeStack_MotorTask;
    unsigned long freeStack_ResistanceCtrlTask;
    unsigned long freeStack_UartTask;
#endif
} SystemRun_T;
extern SystemRun_T SystemRunData;
////////////////***********************************////////////////
///系统运行时数据
typedef struct _UartManger {
    uint8_t ByteSize;  // 数据位
    uint8_t StopBits;  // 停止位
    char Parity[2];    // 校验位
    uint32_t BaudRate; // 波特率
} UartManger_T;
typedef struct _ManagerEEprom {
    uint8_t RunSign;
    char DevId[7];            // 6 位表号
    float KCarLen;            // 轴距传感器修正系数
    float KDegreeSpeed;       // 轴速传感器修正系数
    float KResistanceFront;   // ADC 反拖力修正系数
    float KResistanceBack;    // ADC 反拖力修正系数
    float PwmBasePoint;       // pwm 固定误差点
    int16_t BResistanceFront; // ADC 反拖力修正截距
    int16_t BResistanceBack;  // ADC 反拖力修正截距
    uint8_t ModBusSubAddr;    // 485从机地址
    uint16_t StartCarLen;     // 轴距的初始值
    UartManger_T Uart1Manger;
    UartManger_T Uart4Manger;
    UartManger_T Uart5Manger;
    // 转动惯量参数
    // 寄生功率参数
} ManagerEEprom;
extern ManagerEEprom SystemManager;
////////////////***********************************//////////////// 系统管理参数
#define ID_OF_CTRL_SUSPEND_DEFINED
typedef enum {
    UsDelayFun = 0,
    I2cReadFun,
    I2cWriteFun,
    SendByUartFun,
    ReBootSystem,
} IDOfCtrlSuspend;
////////////////***********************************//////////////// 可以关闭调度器的函数ID
typedef struct _ADC_Data_T {
    uint16_t VarCh1;
    uint16_t VarCh2;
} ADC_Data_T;
////////////////***********************************//////////////// ADC 数据
#endif

#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"

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

/*
CheckIwdg  
    (128 * 4 - 96 * 4) = (512 - 384) = 128 + 110 * 4 = 488
EEpromTask
    (128 * 15 - 402 * 4) = (1920 - 1608) = 312 + 110 * 4 = 422
MeasureTask
    (128 * 15 - 216 * 4) = (1920 - 864) = 1056 + 110 * 4 = 1166
MotorTask
    (128 * 15 - 358 * 4) = (1920 - 1432) = 488 + 110 * 4 = 598
ResistanceCtrlTask
    (128 * 15 - 374 * 4) = (1920 - 1496) = 424 + 110 * 4 = 534
UartTask
    (128 * 40 - 646 * 4) = (5120 - 2584) = 2536 + 110 * 4 = 2646
*/