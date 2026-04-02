#include "AT24CXXDataLoader.h"
#include "StrLib.h"
#include "stddef.h"
#ifdef OPEN_FL33LXX_LIB

struct AT24CXX_MANAGER_S* _AT24CXX_Manager_NET = NULL;
#ifndef __AT24C0XXX_H
AT24CXX_MANAGER_T AT24CXX_Manager_NET;
#endif
void AT24CXXLoader_Init(void) {
    _AT24CXX_Manager_NET = &AT24CXX_Manager;
    AT24CXX_Manager_NET.Time_Data.year = 0x19;
    AT24CXX_Manager_NET.Time_Data.month = 0x10;
    AT24CXX_Manager_NET.Time_Data.day = 0x01;
    AT24CXX_Manager_NET.Time_Data.hour = 0x06;
    AT24CXX_Manager_NET.Time_Data.minute = 0x30;
    AT24CXX_Manager_NET.Time_Data.second = 0x59;
    AT24CXX_Manager_NET.Time_Data.week = getDayOfWeek(0x2020, 0x01, 0x01);
}
bool checkTimeFrom(FL_RTC_InitTypeDef InputTimeData) {
    if (!Check_Time_ByHEX(InputTimeData.hour, InputTimeData.minute, InputTimeData.second)) {
        return false; // 时间错误
    }
    if (!Check_Date_ByHEX((InputTimeData.year >> 16), InputTimeData.year, InputTimeData.month, InputTimeData.day)) {
        return false; // 时间错误
    }
    return ((InputTimeData.year & 0x00FF) < 0x0019 ? false : true);
}
// #include "Display.h"
// AT 参数初始化
// 初始化 Get_Module_Data
void setNetArgumentInit(void (*UserShowdownNowDev)(void)) {
    // Get_Module_Data.CtrlDev_ReceiveCount = 0;
    // Get_Module_Data.CtrlDev_SendCount = 0;
    // 初始化 SetTime 任务
    // 初始化 SetLPTime 任务
    // 初始化 RTC_TASK 任务
    return;
}
#endif
