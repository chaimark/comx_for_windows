#ifndef __UPDATA_H__
#define __UPDATA_H__

#define OPEN_FLASH_256_LIB
#define ISBootLoader 0
// true
// false
#include "PublicLib_No_One.h"
#ifdef OPEN_FLASH_256_LIB
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #define MyPrintf(...)  printf(__VA_ARGS__)
#define MyPrintf(...) HAL_Delay(10)

#if (ISBootLoader == 0)
#include "../../WorkLib/Inc/cJson.h"
#include "../../WorkLib/Inc/NumberBaseLib.h"
#endif

// PAGE_SIZE == 512
#ifdef FL_FLASH_PAGE_SIZE_BYTE
#define PAGE_SIZE FL_FLASH_PAGE_SIZE_BYTE
#else
#define PAGE_SIZE 512
#endif
int flash_write_page(uint32_t addr, uint8_t *buf);
void flash_read_page(uint32_t addr, uint8_t *buf);

// #define FM33LC026_FLASH_EN // 选择打开 FM33LC026 的 flash
#ifndef OPEN_FLASH_256_LIB
// Flash Size == 128k
#define UPDATA_PAGE_BEGIN 0x11C00
#define UPDATA_PAGE_END 0x1F800
#define UPDATA_PAGE_SIGN 0x1F800
#else
// Flash Size == 256k
#define UPDATA_PAGE_BEGIN 0x21C00
#define UPDATA_PAGE_END 0x3F800
#define UPDATA_PAGE_SIGN 0x3F800
#endif

#define UPDATA_SIGN 0xaa55aa55

// 地址偏移
#define UPDATA_MCU_BASE 0x08000000
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
