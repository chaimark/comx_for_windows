#ifndef __PUBLICLIB_NO_ONE_H
#define __PUBLICLIB_NO_ONE_H

// #define OPEN_AT_CMD_DEBUG
// #define OPEN_LOWPWER_DEBUG
// #define OPEN_FL33LXX_LIB
#include "StrLib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef OPEN_FL33LXX_LIB
#include "Define.h"
#include "fm33lc0xx_fl.h"
#endif
#ifdef configMAX_TASK_NAME_LEN
#include "FreeRTOS.h"
#include "Tickless_Hook.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#endif

extern void FL_DelayUs(uint32_t nus);
#define IncludeDelayMs FL_DelayMs

#define ATCMD_MAXNUMBER 1
#define AT24CXX_Manager_NET (*_AT24CXX_Manager_NET)

extern void          IPstrToHexArray(strnew IpHex, const char* Ipstr);
extern unsigned char get_CheckSum(unsigned char data[], int length);
// #define TimeSpeedNum 37 // 设置 1 秒等价与 TimeSpeetNum 秒
#ifdef TimeSpeedNum
extern void TimeSpeed(void);
#endif

extern void BCD_To_String(strnew str, strnew bcd);
extern void WGID_String_To_BCD(strnew bcd, strnew str);

//////////////////////////////////////////////////////


#endif
