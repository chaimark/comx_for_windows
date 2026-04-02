#include "PublicLib_No_One.h"
#include "NumberBaseLib.h"
#include <stdint.h> // 标准整数类型定义
#ifdef OPEN_FL33LXX_LIB
#include "BSTim.h"
#include "fm33lc0xx_fl.h"
#endif
#ifdef configMAX_TASK_NAME_LEN
#include "FreeRTOS.h" // FreeRTOS 基本定义
#include "task.h"     // FreeRTOS 任务管理函数

void FL_DelayUs(uint32_t nus) {
    uint32_t ticks;
    uint32_t told, tnow, reload, tcnt = 0;

    reload = SysTick->LOAD;                    //获取重装载寄存器值
    ticks = nus * (SystemCoreClock / 1000000); //计数时间值

    vTaskSuspendAll();   //阻止OS调度，防止打断us延时
    told = SysTick->VAL; //获取当前数值寄存器值（开始时数值）
    while (1) {
        tnow = SysTick->VAL;
        //当前值不等于开始值说明已在计数
        if (tnow != told) {
            if (tnow < told) {                //当前值小于开始数值，说明未计到0
                tcnt += told - tnow;          //计数值=开始值-当前值
            } else {                          //当前值大于开始数值，说明已计到0并重新计数
                tcnt += reload - tnow + told; //计数值=重装载值-当前值+开始值
            }
            told = tnow;       //更新开始值
            if (tcnt >= ticks) //时间超过/等于要延迟的时间,则退出.
                break;
        }
    }
    xTaskResumeAll(); //恢复OS调度
}
#endif
// SystemCoreClock为系统时钟(system_stmf4xx.c中)，通常选择该时钟作为
// systick定时器时钟，根据具体情况更改
#ifdef TimeSpeedNum
void TimeSpeed(void) {
    int TempNowHour = anyBaseToAnyBase((uint64_t)NowHour, 10, 16) * 60 * 60;
    int TempNowMinute = anyBaseToAnyBase((uint64_t)NowMinute, 10, 16) * 60;
    int TempNowSecond = anyBaseToAnyBase((uint64_t)NowSecond, 10, 16);

    int TempSec = TempNowHour + TempNowMinute + TempNowSecond;
    TempSec += TimeSpeedNum;

    TempNowHour = anyBaseToAnyBase((TempSec / 3600) % 24, 16, 10);
    TempNowMinute = anyBaseToAnyBase(((TempSec % 3600) / 60) % 60, 16, 10);
    TempNowSecond = anyBaseToAnyBase((TempSec % 60) % 60, 16, 10);

    if ((TempNowHour > NowHour) || (NowHour > 0x24)) {
        NowHour = TempNowHour;
        NowMinute = 0;
        NowSecond = 0;
    } else if ((TempNowMinute > NowMinute) || (NowMinute > 0x59)) {
        NowMinute = TempNowMinute;
        NowSecond = 0;
    } else if ((TempNowHour == 0) && (NowHour == 0x23)) {
        NowHour = 0;
        NowMinute = 0;
        NowSecond = 0;
    } else {
        NowSecond = TempNowSecond;
    }

    char setStrOfTime[32] = {0};
    getStrUserTime(RTC_Date, setStrOfTime);
    setRtcDate(NEW_NAME(setStrOfTime), false); // 设置时间
}
#endif

void IPstrToHexArray(strnew IpHex, const char* Ipstr) { // IP字符串转16进制
    char IP_String[] = {"255.255.255.255.."};
    memset(IP_String, 0, strlen("255.255.255.255.."));
    memcpy(IP_String, Ipstr, strlen(Ipstr));
    catString(IP_String, ".", strlen("255.255.255.255.."), 1); //字符串拼接
    char  Str[4] = {0};
    char* P_piont = NULL;
    char* Head = IP_String;
    int   temp = 0;
    for (int i = 0; i < 4; i++) {
        memset(Str, 0, 3);
        if ((P_piont = strchr(Head, '.')) != NULL) {
            *P_piont = '\0';
            memcpy(Str, Head, strlen(Head));
            *P_piont = '.';
            Head = P_piont + 1;
            temp = doneAsciiStrToAnyBaseNumberData(Str, 16); //字符串转任意进制数
            IpHex.Name._char[i] = (unsigned char)temp;
        }
    }
}

unsigned char get_CheckSum(unsigned char data[], int length) {
    unsigned char ResCs = 0;
    int           i = 0;
    for (i = 0; i < length; i++) {
        ResCs += data[i];
    }
    return ResCs;
}
// BCD网关数组转字符串 （bcd 必须沾满）
void BCD_To_String(strnew str, strnew bcd) {
    if (bcd.MaxLen * 2 >= str.MaxLen) { // str 太小, 不够
        return;
    }
    HEX2ToASCII(bcd.Name._char, bcd.MaxLen, str.Name._char, str.MaxLen);
}
// 字符串网关转BCD数组 String < 13
void WGID_String_To_BCD(strnew bcd, strnew str) {
    unsigned char Len = str.getStrlen(&str);
    char          change_char[13] = {0};
    copyString(change_char, str.Name._char, ARR_SIZE(change_char), Len);
    swapStr((char*)change_char, Len);
    if ((Len % 2) != 0) {
        change_char[strlen((char*)change_char)] = '0';
        Len++;
    }
    swapStr((char*)change_char, Len);
    ASCIIToHEX2((char*)change_char, strlen((char*)change_char), (char*)change_char, (Len / 2));
    for (int i = 0; ((i < (Len / 2)) && (i < bcd.MaxLen)); i++) {
        bcd.Name._char[i] = change_char[i];
    }
}
