#include "UpData.h"
#ifdef OPEN_FL33LXX_LIB
#include "mf_config.h"

// FL_FLASH_PAGE_SIZE_BYTE   512 页大小
// 0 ~ 63  块
// 0 ~ 255 页
updata_param_t updata_param;
UpdataDataTemp UpdataData = {0};

// flash 检查 （len%4 * 4） 字节的数据
static int flashCheckReadPageData(uint32_t addr, uint8_t* buf, int len) {
    uint32_t  TempHex = 0;
    uint32_t* PNote = (uint32_t*)addr;
    for (size_t i = 0; i < PAGE_SIZE / sizeof(uint32_t); i++) {
        TempHex = (*PNote);
        if (((uint32_t*)buf)[i] != TempHex) {
            return -1;
        }
        PNote++;
    }
    return 0;
}
// 把 buf 开始的数据，写入到 addr 开始的地址中, 并读出校验
int flash_write_page(uint32_t addr, uint8_t* buf) {
    for (int i = 0; i < 3; i++) {
        if (FL_FLASH_PageErase(FLASH, addr) == FL_FAIL) {
            break;
        }
        if (FL_FLASH_Program_Page(FLASH, addr / 512, (uint32_t*)buf) == FL_FAIL) {
            break;
        }
        // 校验
        if (flashCheckReadPageData(addr, buf, PAGE_SIZE) < 0) {
            break;
        }
        return 0;
    }
    return -1;
}
void flash_read_page(uint32_t addr, uint8_t* buf) {
    uint32_t  TempHex = 0;
    uint32_t* PNote = (uint32_t*)addr;
    for (size_t i = 0; i < PAGE_SIZE / sizeof(uint32_t); i++) {
        TempHex = (*PNote);
        buf[i * 4 + 0] = (TempHex & 0x000000FF) >> 0;
        buf[i * 4 + 1] = (TempHex & 0x0000FF00) >> 8;
        buf[i * 4 + 2] = (TempHex & 0x00FF0000) >> 16;
        buf[i * 4 + 3] = (TempHex & 0xFF000000) >> 24;
        PNote++;
    }
}

/////////////////////////////////////
/////////////////////////////////////

// 读标志扇区
void updataReadSign(void) {
    uint8_t* p = (uint8_t*)(&updata_param);
    for (size_t i = 0; i < sizeof(updata_param_t); i++)
        p[i] = *(uint8_t*)(UPDATA_PAGE_SIGN + i);
}
// 写标志扇区
void updataWriteSign(void) {
    memset(UpdataData.Page8Buff, 0xFF, PAGE_SIZE);
    memcpy(UpdataData.Page8Buff, &updata_param, sizeof(updata_param_t));
    flash_write_page(UPDATA_PAGE_SIGN, UpdataData.Page8Buff);
    memset((char*)&updata_param, 0, sizeof(updata_param_t));
    updataReadSign();
}
// 初始化标志区
void updataInit(void) {
    memset(UpdataData.Page8Buff, 0xFF, PAGE_SIZE);
    UpdataData.NowLen_Page8Buff = 0;
    memset(&updata_param, 0, sizeof(updata_param));
    updataWriteSign();
}
#if ISBootLoader
// 检查是否需要升级
int updataCheck(void) {
    updataReadSign();
    uint8_t Check_CS_Num = 0;
    if (updata_param.sign != UPDATA_SIGN || updata_param.pageNum < 10) {
        return -1;
    }
    MyPrintf("updata find page = %d\r\n", updata_param.pageNum);
    // 有已经更新的待运行的程序，需要校验
    for (int i = 0; i < updata_param.pageNum; i++) {
        memset(UpdataData.Page8Buff, 0, PAGE_SIZE);                           // 初始化读取空间
        flash_read_page(UPDATA_PAGE_BEGIN + (i * 512), UpdataData.Page8Buff); // 读
        if (flashCheckReadPageData(UPDATA_PAGE_BEGIN + (i * 512), UpdataData.Page8Buff, PAGE_SIZE) < 0) {
            return -1; // flash 与 buff 不一致读取失败
        }
        // 累加校验和
        for (int i = 0; i < PAGE_SIZE; i++) {
            Check_CS_Num += UpdataData.Page8Buff[i]; // 累加第一页的校验和
        }
    }
    if (Check_CS_Num != updata_param.checkSum) {
        return -1;
    }
    return 0;
}
// copy 程序到运行区
int updataCopyProgram(void) {
    uint32_t addr = 0;
    int      Flag = 0; //升级标记
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < updata_param.pageNum; i++) {
            // 读
            flash_read_page(UPDATA_PAGE_BEGIN + (i * 512), UpdataData.Page8Buff);

            // 写
            addr = UPDATA_MCU_OFFSET + (i * 512);
            if (flash_write_page(addr, UpdataData.Page8Buff) < 0) {
                MyPrintf("updata page %04x program fail\r\n", addr);
                Flag = -1;
                break;
            }

            MyPrintf("updata addr %04x sucess\r\n", addr);
        }
        if (Flag != -1) {
            break;
        }
    }
    updataInit();
    return Flag;
}
#else
uint8_t UpdataCsDeart; // 校验差
// 读取 flash 数据 ==> 缓存区
void readFlashDataToUpdataBuff(uint8_t PageNum) {
    flash_read_page((UPDATA_PAGE_BEGIN + PageNum * PAGE_SIZE), UpdataData.Page8Buff);
}
// 写入 缓存区数据 ==> flash
void writeUpdataBuffDataToFlash(uint8_t PageNum) {
    flash_write_page((UPDATA_PAGE_BEGIN + PageNum * PAGE_SIZE), UpdataData.Page8Buff);
}
// 计算包所在的 flash 页
int ComputeNeedPage(int PackNum, int PackLen) {
    return ((PackNum * PackLen) / PAGE_SIZE);
}
// 为不满 512 的缓存空间 补充 0xff
void addHex_FF_ToBuff(void) {
    memset(&UpdataData.Page8Buff[UpdataData.NowLen_Page8Buff], 0xFF, PAGE_SIZE - UpdataData.NowLen_Page8Buff);
    UpdataData.NowLen_Page8Buff = PAGE_SIZE;
}
// 判断当前包所在页面的剩余空间
int NowPackIsGotoNextPage(int NowPageNum, int NowPackNum) {
    if (NowPackNum * UpdataData.PackLen > (NowPageNum + 1) * PAGE_SIZE) {
        return -1; // 确保页面号与 包序号正确
    }
    return ((NowPageNum + 1) * PAGE_SIZE - NowPackNum * UpdataData.PackLen); // 当前页剩余空间
}
// 把当前包存入缓存区，如果缓存区满，则写 flash
// 备注： NowPackNum == -1 则将剩下缓存区写入 flash
int SaveUpdataToPage8Buff(int NowPageNum, int NowPackNum, strnew NowCodeHex) {
    if (NowPageNum != UpdataData.NowPageNum) { // 不同页, 写 flash
        writeUpdataBuffDataToFlash(UpdataData.NowPageNum);
        readFlashDataToUpdataBuff(NowPageNum);
    }
    int RestLen = NowPackIsGotoNextPage(NowPageNum, NowPackNum);
    if (RestLen < 0) {
        return 1;
    }
    if (RestLen < NowCodeHex.MaxLen) { // 需要跳页
        UpdataData.NowLen_Page8Buff = PAGE_SIZE - RestLen;
        memcpy(&UpdataData.Page8Buff[UpdataData.NowLen_Page8Buff], NowCodeHex.Name._char, RestLen);
        writeUpdataBuffDataToFlash(NowPageNum);
        readFlashDataToUpdataBuff(NowPageNum + 1);
        memset(UpdataData.Page8Buff, 0xFF, PAGE_SIZE);
        memcpy(UpdataData.Page8Buff, NowCodeHex.Name._char + RestLen, NowCodeHex.MaxLen - RestLen);
        UpdataData.NowPageNum = (++NowPageNum);
        UpdataData.NowLen_Page8Buff = NowCodeHex.MaxLen - RestLen;
    } else {                                                      // 不需要跳页
        int Addr = (NowPackNum * UpdataData.PackLen) % PAGE_SIZE; // 当前包需要保存到缓存区的角标
        memcpy(&UpdataData.Page8Buff[Addr], NowCodeHex.Name._char, NowCodeHex.MaxLen);
        UpdataData.NowLen_Page8Buff = Addr + NowCodeHex.MaxLen;
    }
    UpdataData.NowPageNum = NowPageNum; // 更新页码
    if (NowCodeHex.MaxLen == UpdataData.PackLen) {
        return 2;
    }
    for (int i = 0; i < (PAGE_SIZE - UpdataData.NowLen_Page8Buff); i++) {
        UpdataCsDeart += 0xFF;
    }
    addHex_FF_ToBuff(); // 最后一页需要补 0xFF
    return 2;
}
// "data":{"PackLen":128}
// "data":{"NowPackNum":0,"Code":"11223344556677889900","CsCheckNum":114}
// "data":{"upDataFlag":true,"CsCheckNum":90,"pageNum":10}
int UpData_Receive_Hex(JsonObject BinCode) {
    newString(CodeStr, PAGE_SIZE * 2);
    int     FlagCodeNum; // 返回码
    uint8_t checkSum = 0;
    if (UpdataData.Sign != 0xB2) {
        if (BinCode.isJsonNull(&BinCode, "PackLen") < 0) {
            FlagCodeNum = 0;
            goto OverSub;
        }
    }
    if (BinCode.getBool(&BinCode, "upDataFlag")) {
        updata_param.sign = UPDATA_SIGN;
        updata_param.checkSum = BinCode.getInt(&BinCode, "CsCheckNum") + UpdataCsDeart;
        updata_param.pageNum = BinCode.getInt(&BinCode, "pageNum");
        updataWriteSign(); // 写标记区
        FlagCodeNum = 3;
        goto OverSub;
    }
    // 存在版本信息，准备升级，或重新升级
    if (BinCode.isJsonNull(&BinCode, "PackLen") >= 0) {
        updataInit();
        if ((unsigned char)UpdataData.Sign == 0) { // 第一次升级
            UpdataData.PackLen = BinCode.getInt(&BinCode, "PackLen");
        } else { // 重新升级
            UpdataData.PackLen = BinCode.getInt(&BinCode, "PackLen");
        }
        UpdataData.Sign = 0xB2;
        UpdataData.NowPageNum = 0;
        UpdataData.NowLen_Page8Buff = 0;
        memset(UpdataData.Page8Buff, 0xFF, PAGE_SIZE);
        FlagCodeNum = 0;
        goto OverSub;
    }
    BinCode.getString(&BinCode, "Code", CodeStr);
    CodeStr.MaxLen = ASCIIToHEX2(CodeStr.Name._char, CodeStr.getStrlen(&CodeStr), CodeStr.Name._char,
                                 CodeStr.getStrlen(&CodeStr) / 2);
    // 计算校验
    for (int i = 0; i < CodeStr.MaxLen; i++) {
        checkSum += CodeStr.Name._char[i];
    }
    FlagCodeNum = BinCode.getInt(&BinCode, "CsCheckNum");
    if (checkSum != FlagCodeNum) {
        FlagCodeNum = 1;
        goto OverSub;
    }
    UpdataData.NowPackNum = BinCode.getInt(&BinCode, "NowPackNum"); // 获取包序号
    FlagCodeNum = SaveUpdataToPage8Buff(ComputeNeedPage(UpdataData.NowPackNum, UpdataData.PackLen),
                                        UpdataData.NowPackNum, CodeStr);
OverSub:
    if (UpdataData.NowLen_Page8Buff == PAGE_SIZE) {
        writeUpdataBuffDataToFlash(UpdataData.NowPageNum);
        UpdataData.NowLen_Page8Buff = 0;
    }
    return FlagCodeNum; // 返回标志位
}
#endif
#endif
