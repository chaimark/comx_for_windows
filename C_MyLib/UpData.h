#ifndef __UPDATA_H__
#define __UPDATA_H__

#define ISBootLoader 1
// true
// false
#include "PublicLib_No_One.h"
#ifdef OPEN_FL33LXX_LIB
#include "fm33lc0xx_fl_flash.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #define MyPrintf(...)  printf(__VA_ARGS__)
#define MyPrintf(...) FL_DelayMs(10)

#if (ISBootLoader == 0)
#include "../Interflow/JsonDataAnalyzeLib.h"
#include "../Interflow/NumberBaseLib.h"
#endif

// PAGE_SIZE == 512
#ifdef FL_FLASH_PAGE_SIZE_BYTE
#define PAGE_SIZE FL_FLASH_PAGE_SIZE_BYTE
#else
#define PAGE_SIZE 512
#endif
int flash_write_page(uint32_t addr, uint8_t* buf);
void flash_read_page(uint32_t addr, uint8_t* buf);

// #define FM33LC026_FLASH_EN // 选择打开 FM33LC026 的 flash
#ifdef FM33LC026_FLASH_EN
/***********FM33LC026****************/
#define UPDATA_PAGE_SIGN 0x1F000
#define UPDATA_PAGE_BEGIN 0x11800
#define UPDATA_PAGE_END 0x1F000
#else
/***********FM33LC046****************/
#define UPDATA_PAGE_SIGN 0x3E000
#define UPDATA_PAGE_BEGIN 0x23000
#define UPDATA_PAGE_END 0x3E000
#endif

#define UPDATA_SIGN 0xaa55aa55

// 地址偏移
#define UPDATA_MCU_BASE 0
#define UPDATA_MCU_OFFSET 0x4000 // 前 16k 是bootloader

typedef struct _UpdataFlag {
    uint32_t sign;    // 标志位
    uint32_t pageNum; // 程序占用的 flash 页数
    uint8_t checkSum; // 校验和
} UpdataFlag;
extern UpdataFlag UpdataParam;

typedef struct _UpdataDataTemp {
    char Sign;
    int PackLen;
    int NowPackNum; // 接收到的包序号
    int NowPageNum; // 当前包所在的页号
    uint8_t Page8Buff[PAGE_SIZE];
    int NowLen_Page8Buff;
} UpdataDataTemp;
extern UpdataDataTemp UpdataData;

extern void updataInit(void);
extern void updataReadSign(void);
extern void updataWriteSign(void);

#if ISBootLoader
extern int updataCheck(void);
int updataCopyProgram(void);
#else
extern int UpData_Receive_Hex(JsonObject BinCode);
#endif

#endif
#endif
