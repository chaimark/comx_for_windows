#ifndef AT24XXXDATALOADER_H
#define AT24XXXDATALOADER_H

#include "NumberBaseLib.h"
#include <stdbool.h>
#ifdef OPEN_FL33LXX_LIB
#include "AT24C02.h"

#ifndef __AT24C0XXX_H
typedef struct {
    /** 年 */
    uint32_t year;
    /** 月 */
    uint32_t month;
    /** 日 */
    uint32_t day;
    /** 周 */
    uint32_t week;
    /** 时 */
    uint32_t hour;
    /** 分 */
    uint32_t minute;
    /** 秒 */
    uint32_t second;

} FL_RTC_InitTypeDef;
#else
#include "RTC.h"
#endif

/*
电信 Band5
移动 Band8
联通 Band3
*/
// 需要引入外部 AT24CXX_MANAGER_S 定义
#ifndef __AT24C0XXX_H
typedef struct AT24CXX_MANAGER_S {
    // 32字节
    char     Sing;               // 初始化标志
    char     VER[4];             // 版本号
    char     Save_Working_Mode;  // 工作模式（权限）
    char     MeterID[4];         // 表号
    char     NET_Remote_Url[42]; // 目标地址
    uint16_t NET_Remote_Port;    // 目标端口
    char     DaysNumberOfCCLK;   // 间隔校时的天数
    struct {
        uint32_t SendIntervalDay; // 当天发送的周期 (单位是 s) 1440 分钟 = 1天 = 86400 秒
        uint8_t  SendStartHour;   // 当天发送的起始点
        struct {
            uint16_t StartDay;          // 某模式起始日（如：第150日; 1月1日为第1日）
            uint8_t  SendInterval;      // 某模式下的间隔周期 (单位是 day)
            uint8_t  OneDayMaxSendTime; // 某模式下的最大发送次数
        } SendFlagMode[4]; // 当月发送标记模式 (默认：0:防锁模式, 1:频发模式, 2:保底模式)
    } SendManageObj;
    struct {
        char _Module_SIM_BAND;
        char _Module_SIM_ICCID[21];
        char _Module_SIM_IMSI[21];
        char _Module_DEV_IMEI[21];
        // char _Module_DEV_SNR;
        char _Module_DEV_CSQ;
        char _Module_DEV_RSRP;
        char _Module_DEV_RSRQ;
        char _Module_Socket_Num;
    } Get_Module_Data;
    FL_RTC_InitTypeDef Time_Data;
    bool               NetCheckENableFlag; // 网络检测使能标志
} AT24CXX_MANAGER_T;
extern AT24CXX_MANAGER_T AT24CXX_Manager;
#endif

extern AT24CXX_MANAGER_T* _AT24CXX_Manager_NET;
extern void               AT24CXXLoader_Init(void);
extern bool               checkTimeFrom(FL_RTC_InitTypeDef InputTimeData);
#ifdef _Module_EEpromTime
extern void setIsWriteEEprom(uint8_t UserFlagType);
#endif

extern void setNetArgumentInit(void (*UserShowdownNowDev)(void));
#endif
#endif
