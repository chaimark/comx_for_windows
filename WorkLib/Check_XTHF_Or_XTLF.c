#include "Check_XTHF_Or_XTLF.h"
#include "PublicLib_No_One.h"
#ifdef OPEN_FL33LXX_LIB
#include "Define.h"
#include "fm33lc0xx_fl.h"
#include "mf_config.h"

static void check_XTClock_isOk(void);

Check_XTHF_Or_XTLF XTClock_CheckTask = {
    .is_XTClock_Error = false,
    .UserTask = NULL,
};
/**********************************************************************/
/**********************************************************************/
#ifdef XTLF_FAIL
// 外部低速晶振检测初始化
void XTLF_Config_Init(void) {
    FL_NVIC_ConfigTypeDef XTLF;
    FL_FDET_EnableIT_XTLFFail(); // 使能 XTLF 失效中断
    XTLF.preemptPriority = 0x0001;
    FL_NVIC_Init(&XTLF, LFDET_IRQn); // 初始化NVIC, 使用上面配置的结构体变量
}
// 低频时钟检测中断
void LFDET_IRQHandler(void) {
    FL_FDET_DisableIT_XTLFFail(); // 关闭 XTLF 失效中断
    FL_FDET_ClearFlag_XTLFFail(); // 处理 XTLF 失效
    XTClock_CheckTask.is_XTClock_Error += XTLF_FAIL;
    // 切换时钟源, 内部低速时钟源(LPOSC)作为低速时钟源
    FL_RCC_SetLSCLKClockSource(FL_RCC_LSCLK_CLK_SOURCE_LPOSC);
    MF_RTC_1S_Init();
    // 创建一个RTC定时器任务 CheckPWMOfClock
    MIN_TASK.InitSetTimeTask(CheckPWMOfClock, MinToSec(5), check_XTClock_isOk);
}
#endif
/**********************************************************************/
/**********************************************************************/
#ifdef XTHF_FAIL
// 外部高速晶振检测初始化
void XTHF_Config_init(void) {
    FL_NVIC_ConfigTypeDef XTHF;
    FL_FDET_EnableIT_XTHFFail(); // 使能 XTHF 失效中断
    XTHF.preemptPriority = 0x0001;
    FL_NVIC_Init(&XTHF, HFDET_IRQn); // 初始化NVIC, 使用上面配置的结构体变量
}
// 高频时钟检测中断
void HFDET_IRQHandler(void) {
    FL_FDET_DisableIT_XTHFFail(); // 关闭 XTHF 失效中断
    FL_FDET_ClearFlag_XTHFFail(); // 处理 XTHF 失效
    XTClock_CheckTask.is_XTClock_Error += XTHF_FAIL;
    // 切换时钟源, 内部低速时钟源(LPOSC)作为低速时钟源
    FL_RCC_SetLSCLKClockSource(FL_RCC_LSCLK_CLK_SOURCE_LPOSC);
    MF_RTC_1S_Init();
    // 创建一个RTC定时器任务 CheckPWMOfClock
    MIN_TASK.InitSetTimeTask(CheckPWMOfClock, MinToSec(5), check_XTClock_isOk);
}
#endif
/**********************************************************************/
/**********************************************************************/
void Config_Init_XTHF_And_XTLF(void) {
#ifdef XTLF_FAIL
    XTLF_Config_Init();
#endif
#ifdef XTHF_FAIL
    XTHF_Config_init();
#endif
}
#ifdef XT_CLOCK_OK
void check_XTClock_isOk(void) {
    MIN_TASK.InitSetTimeTask(CheckPWMOfClock, MinToSec(5), check_XTClock_isOk);
    if (XTClock_CheckTask.is_XTClock_Error == XT_CLOCK_OK) {
        MIN_TASK.CloseTask(CheckPWMOfClock);
        return; // 时钟正常, 不需要检查
    }
    uint8_t TempErrorNum = 0;
    uint32_t (*Temp_FL_FDET_IsActiveFlag_XTClock_Fail)(void);
#ifdef XTLF_FAIL
    XTLF_Config_Init();
    TempErrorNum = XTLF_FAIL;
    Temp_FL_FDET_IsActiveFlag_XTClock_Fail = FL_FDET_IsActiveFlag_XTLFFail;
#endif
#ifdef XTHF_FAIL
    XTHF_Config_init();
    TempErrorNum = XTHF_FAIL;
    Temp_FL_FDET_IsActiveFlag_XTClock_Fail = FL_FDET_IsActiveFlag_XTHFFail;
#endif
    // 检测高速时钟是否正常
    if (!Temp_FL_FDET_IsActiveFlag_XTClock_Fail()) {
        XTClock_CheckTask.is_XTClock_Error -= TempErrorNum;
        if (XTClock_CheckTask.UserTask != NULL) {
            XTClock_CheckTask.UserTask();
        }
        MF_Clock_Init();
        FL_RCC_SetLSCLKClockSource(FL_RCC_LSCLK_CLK_SOURCE_XTLF); // 切换时钟源, 外部时钟
        char TimeStr[20] = {0};
        getStrNowDataAndTimeByRTC(TimeStr);
        MF_RTC_1S_Init();
        setRtcDate(NEW_NAME(TimeStr), false);
        Config_Init_XTHF_And_XTLF();
        MIN_TASK.CloseTask(CheckPWMOfClock);
        return;
    }
#ifdef XTLF_FAIL
    FL_FDET_DisableIT_XTLFFail(); // 关闭 XTLF 失效中断
    FL_FDET_ClearFlag_XTLFFail(); // 处理 XTLF 失效
#endif
#ifdef XTHF_FAIL
    FL_FDET_DisableIT_XTHFFail(); // 关闭 XTHF 失效中断
    FL_FDET_ClearFlag_XTHFFail(); // 处理 XTHF 失效
#endif
    return;
}
#endif
#endif
