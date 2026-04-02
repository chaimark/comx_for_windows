#include "NumberBaseLib.h"
#include <stdio.h>
#include <stdlib.h>
double BuffToFloatOrDouble(strnew OutBuff, bool IsDouble) {
    union {
        double  TempDouble;
        float   TempFloat;
        uint8_t Buff[8];
    } converter;
    if ((OutBuff.MaxLen > 4) && (!IsDouble)) {
        return 0;
    }
    if ((OutBuff.MaxLen > 8) && (IsDouble)) {
        return 0;
    }
    swapStr(OutBuff.Name._char, OutBuff.MaxLen);
    for (int i = 0; i < OutBuff.MaxLen; i++) {
        converter.Buff[i] = OutBuff.Name._char[i];
    }
    if (IsDouble) {
        return converter.TempDouble;
    } else {
        return (double)converter.TempFloat;
    }
}
void DoubleOrFloatToBuff(strnew OutBuff, double Number, bool IsDouble) {
    uint8_t BuffLen = 0;
    union {
        double   TempDouble;
        float    TempFloat;
        uint64_t Buff8;
    } converter;
    if (IsDouble) {
        if (OutBuff.MaxLen < 8) {
            return;
        }
        converter.TempDouble = Number;
        BuffLen = 8;
    } else {
        if (OutBuff.MaxLen < 4) {
            return;
        }
        converter.TempFloat = Number;
        BuffLen = 4;
    }
    for (int i = 0; i < BuffLen; i++) {
        OutBuff.Name._char[i] = (uint8_t)(converter.Buff8 >> (i * 8) & 0xFF);
    }
    swapStr(OutBuff.Name._char, BuffLen);
}
/*
// 该库中提到的所有进制数全是直接表示, 比如
// hex(0x125)   //表示 dec(125)
// hex(0x1010)  //表示 bin(1010)
// hex(0x1f)    //表示 hex(0x1f)
*/

// 将数组串转字符串（可以原地转换, 需要注意数组串的长度与字符串的长度, 不是整个数组的长度）
void numberArrayToStrArray(char StrArray[], char NumberArray[], int ArrayMinLen) {
    int Addr = 0;
    do {
        if (NumberArray[Addr] < 10) {
            StrArray[Addr] = NumberArray[Addr] + '0';
        } else if ((NumberArray[Addr] >= 0x0A) && (NumberArray[Addr] <= 0x0F)) {
            StrArray[Addr] = NumberArray[Addr] + 0x37;
        }
        Addr++;
    } while (Addr < ArrayMinLen);
}
// 将字符串转数组串（可以原地转换, 需要注意数组串的长度与字符串的长度, 不是整个数组的长度）
void strArrayToNumberArray(char NumberArray[], char StrArray[], int ArrayMinLen) {
    int Addr = 0;
    do {
        if ((StrArray[Addr] >= '0') && (StrArray[Addr] <= '9')) {
            NumberArray[Addr] = StrArray[Addr] - '0';
        } else if ((StrArray[Addr] >= 'A') && (StrArray[Addr] <= 'Z')) {
            NumberArray[Addr] = StrArray[Addr] - 0x37;
        } else if ((StrArray[Addr] >= 'a') && (StrArray[Addr] <= 'z')) {
            NumberArray[Addr] = StrArray[Addr] - 0x57;
        }
        Addr++;
    } while (Addr < ArrayMinLen);
}
// 任意进制互转
uint64_t anyBaseToAnyBase(uint64_t Number, int IntputBase, int OutputBase) {
    uint64_t ResNumber = Number;
    uint8_t  TempArray[16] = {0}; // uint64_t也只有16位数 0x0000000000000001

    // ============================先转到16进制============================ //
    memset(TempArray, 0, 16); // 清空数组
    // 16进制直接返回
    if (IntputBase != 16) {
        ResNumber = 0;
        // 先将数据转为数组串
        for (int Add_i = 0; Add_i < 16; Add_i++) {
            TempArray[Add_i] = Number & 0x000000000000000F; // 先存低位
            Number = Number >> 4;
        }
        // 根据数组串和该数原来的进制 转换为16进制 如：Dex(0x31)==>Hex(0x1f)
        for (int Add_i = 0; Add_i < 16; Add_i++) {
            uint64_t TempNumber = TempArray[Add_i];
            // 单独计算每个空间对应的数的大小
            for (int i = 0; i < Add_i; i++) {
                TempNumber = TempNumber * IntputBase;
            }
            ResNumber += TempNumber;
        }
        Number = ResNumber; // 先转到16进制
    }

    // ============================再转到任意进制============================ //
    memset(TempArray, 0, 16); // 清空数组
    // 16进制直接返回
    if (OutputBase != 16) {
        ResNumber = 0;
        // 先将数据根据目标进制转为数组串
        for (int Add_i = 0; Add_i < 16; Add_i++) {
            TempArray[Add_i] = Number % OutputBase; // 先存低位
            Number /= OutputBase;
        }
        // 再将数组串组装成直接表示的数
        // 如：hex(0x125) ==> dec(125)
        for (int Add_i = 15; Add_i >= 0; Add_i--) {
            ResNumber = ResNumber << 4;
            Number = TempArray[Add_i] & 0x0F; // 0b:00001111
            ResNumber = ResNumber | Number;
        }
        Number = ResNumber; // 再转到任意进制
    }
    ResNumber = Number;
    return ResNumber;
}
// 任意进制数 转 对应进制数组 返回长度
uint64_t anyBaseNumberToSameArray(uint8_t* Output, int StrSize, uint64_t InputNumber) {
    int8_t   Add_i = 0;
    uint8_t  TempArray[16] = {0}; // uint64_t也只有16位数 0x0000000000000001
    uint64_t OutLen = 0;

    // 将数据转为字符串
    for (Add_i = 0; (Add_i < 16) && (Add_i < StrSize); Add_i++) {
        TempArray[Add_i] = InputNumber & 0x000000000000000F; // 先存低位
        InputNumber = InputNumber >> 4;
    }
    // 计算长度
    OutLen = ((StrSize > 16) ? 16 : StrSize);
    Add_i = ((StrSize > 16) ? 16 : StrSize);
    while ((TempArray[Add_i] == 0) && ((Add_i >= 0) && (Add_i < 16))) {
        OutLen--;
        Add_i--;
    }
    swapStr((char*)TempArray, OutLen);
    for (Add_i = 0; (Add_i < 16) && ((uint64_t)Add_i < OutLen); Add_i++) {
        Output[Add_i] = TempArray[Add_i];
    }
    return OutLen;
}
// 任意进制数组 转 对应进制数 返回数 12345600 ==> 654321
uint64_t anyArrayToSameBaseNumber(uint8_t* InputStr, int StrSize) {
    uint8_t  TempArray[16] = {0}; // uint64_t也只有16位数 0x0000000000000001
    uint64_t ResNumber = 0;
    // 输入的数组长度大于16直接返回
    StrSize = (StrSize > 16 ? 16 : StrSize);
    // copy 输入的数据
    for (int Add_i = 0; Add_i < StrSize; Add_i++) {
        TempArray[Add_i] = InputStr[Add_i];
    }
    // 将数组串组装成直接表示的数
    // 如：hex(0x125) ==> dec(125)
    for (int Add_i = 15; Add_i >= 0; Add_i--) {
        ResNumber = ResNumber << 4;
        uint64_t Number = TempArray[Add_i] & 0x0F; // 0b:00001111
        ResNumber = ResNumber | Number;
    }
    return ResNumber;
}
#include <string.h>
// 任意进制数 转 任意进制数组
int anyBaseNumberToAnyBaseArray(uint64_t Number, int IntputBase, int OutputBase, char OutArray[], int ArrayMaxLen) {
    uint64_t TempNum = anyBaseToAnyBase(Number, IntputBase, OutputBase);
    char     TempArray[16] = {0};
    int      ArrayLen = anyBaseNumberToSameArray((uint8_t*)TempArray, 16, TempNum);
    if (ArrayMaxLen < ArrayLen) {
        return 0;
    }
    memcpy(OutArray, TempArray, ArrayLen);
    return ArrayLen;
}

// 任意进制数组 转 任意进制数 Array:12345600 ==> 12345600
int64_t anyBaseArrayToAnyBaseNumber(char IntArray[], int NumStrNowLen, int IntputBase, int OutputBase) {
    swapStr(IntArray, NumStrNowLen); // 先将数组从从大端模式改为小端
    int64_t TempNum = anyArrayToSameBaseNumber((uint8_t*)IntArray, NumStrNowLen);
    TempNum = anyBaseToAnyBase(TempNum, IntputBase, OutputBase);
    return TempNum;
}

// 单字节数组 转 双字节数组 0x23 --> 0x02 0x03 （原地转换会出现覆盖, 所以不支持原地, 且需要两个 strnew）
int shortChStrToDoubleChStr(strnew inputArray, strnew OutputArray) {
    if (&(inputArray.Name._char) != &(OutputArray.Name._char)) {
        int  ResLen = 0;
        char TempChar[2] = {0};
        if (OutputArray.MaxLen < (inputArray.MaxLen * 2)) {
            return 0;
        }
        for (int i = 0; i < inputArray.MaxLen; i++) {
            memset(TempChar, 0, 2);
            if (anyBaseNumberToSameArray((uint8_t*)TempChar, 2, inputArray.Name._char[i]) == 1) {
                swapStr(TempChar, 2);
            }
            ResLen += 2; // 单字节转两字节
            OutputArray.Name._char[i * 2 + 0] = TempChar[0];
            OutputArray.Name._char[i * 2 + 1] = TempChar[1];
        }
        return ResLen;
    }
    return 0;
}
// 双字节数组 转 单字节数组 0x02 0x03 --> 0x23 （长度足够, 支持原地转换, 需要两个 strnew）
int doubleChStrToShortChStr(strnew inputArray, strnew OutputArray) {
    if (&(inputArray.Name._char) != &(OutputArray.Name._char)) {
        int ResLen = 0;
        if (OutputArray.MaxLen < (inputArray.MaxLen / 2)) {
            return 0;
        }
        for (ResLen = 0; ResLen < (inputArray.MaxLen / 2); ResLen++) {
            OutputArray.Name._char[ResLen] =
                ((inputArray.Name._char[ResLen * 2] & 0x0F) << 4) | (inputArray.Name._char[ResLen * 2 + 1] & 0x0F);
        }
        return ResLen;
    }
    return 0;
}

// 字符串转 任意进制数
int doneAsciiStrToAnyBaseNumberData(char AscArray[], int OutputBase) {
    int NumStrNowLen = 0;
    // 查询符号
    int sign = 1;
    if (*AscArray == '-') {
        sign = -1;
        AscArray++;
    } else if (*AscArray == '+') {
        AscArray++;
    }
    while ((AscArray[NumStrNowLen] >= '0') && (AscArray[NumStrNowLen] <= '9')) {
        NumStrNowLen++;
    }
    newString(NumStr, 12);
    memcpy(NumStr.Name._char, AscArray, NumStrNowLen);
    strArrayToNumberArray(NumStr.Name._char, AscArray, NumStrNowLen);                           // 去掉 0x30
    int NumTemp = anyBaseArrayToAnyBaseNumber(NumStr.Name._char, NumStrNowLen, 10, OutputBase); // 组合成 任意进制数
    return ((sign == 1) ? NumTemp : -NumTemp);
}
// 任意进制数 转 字符串
int doneBaseNumberDataToAsciiStr(char AscArray[], int ArrayMaxLen, int NumberData, int IntputBase) {
    uint64_t TempNum = anyBaseToAnyBase(NumberData, IntputBase, 10);                      // 先转到 10进制
    int AscArrayLen = anyBaseNumberToSameArray((uint8_t*)AscArray, ArrayMaxLen, TempNum); // 10进制 转对应数组
    numberArrayToStrArray(AscArray, AscArray, AscArrayLen);                               // 数组串 转 字符串
    return AscArrayLen;
}

// 字符串 转 double
double doneAsciiToDouble(char AscArray[]) {
    return atof(AscArray);
}
// double 转 字符串
void doneDoubleToAscii(char AscArray[], const char From[], double InputData) {
    sprintf(AscArray, From, InputData);
}
// 读取某位 返回对应位的 bool 值
bool readDataBit(uint64_t InputNumber, int8_t BitNumber) {
    if (BitNumber < 0 || BitNumber >= 64) {
        return false;
    }
    uint64_t mask = (1ULL << BitNumber);
    return ((InputNumber & mask) != 0);
}

// 设置某位 返回是否设置成功
uint64_t setDataBit(uint64_t InputNumber, int8_t BitNumber, bool Value) {
    if (BitNumber < 0 || BitNumber >= 64) {
        return InputNumber;
    }
    uint64_t mask = (1ULL << BitNumber);
    if (Value) {
        InputNumber |= mask;
    } else {
        InputNumber &= ~mask;
    }
    return InputNumber;
}

// 外用接口（不支持原地转换）
int HEX2ToASCII(char* hex, int hex_len, char* asc, int asc_len) {
    if (hex_len * 2 > asc_len) { // asc_len 太小, 不够
        return 0;
    }
    // HEX TO STR-------------------
    newstrobj(IDStr, 1);
    newstrobj(IDHex, 1);
    IDHex.Name._char = hex;
    IDHex.MaxLen = hex_len;
    IDStr.Name._char = asc;
    IDStr.MaxLen = asc_len;
    int TempLen = 0;
    // 单字节数组 转 双字节数组 0x23 --> 0x02 0x03
    if ((TempLen = shortChStrToDoubleChStr(IDHex, IDStr)) < asc_len) {
        IDStr.MaxLen = TempLen;
    }
    // 将数组串转字符串
    numberArrayToStrArray(IDStr.Name._char, IDStr.Name._char, IDStr.MaxLen);
    IDStr.Name._char[2 * IDHex.MaxLen] = '\0';
    return IDStr.MaxLen;
    // HEX TO STR-------------------
}

// 外用接口（支持原地转换：注, 非原地转换时输入的字符串会被破坏）
int ASCIIToHEX2(char* asc, int asc_len, char* hex, int hex_len) {
    if (hex_len < asc_len / 2) { // hex_len 太小, 不够
        return 0;
    }
    // STR TO HEX-------------------
    newstrobj(IDStr, 1);
    newstrobj(IDHex, 1);
    IDStr.Name._char = asc;
    IDStr.MaxLen = asc_len; // 字符串长度
    IDHex.Name._char = hex;
    IDHex.MaxLen = hex_len;                                                  // 最大空间
    strArrayToNumberArray(IDStr.Name._char, IDStr.Name._char, IDStr.MaxLen); // 将字符串转数组串
    int Len = doubleChStrToShortChStr(IDStr, IDHex); // 双字节数组 转 单字节数组 0x02 0x03 --> 0x23
    return Len;
    // STR TO HEX-------------------
}

uint16_t U8_Connect_U8(uint8_t H_Part_Input, uint8_t L_Part_Input) {
    uint16_t H_Part = H_Part_Input;
    uint16_t L_Part = L_Part_Input;
    return (((H_Part & 0x00FF) << 8) | (L_Part & 0x00FF));
}

float getPartOfSetPointOnRing(uint32_t SetPoint, uint32_t Min_Ring, uint32_t Max_Ring) {
    if ((SetPoint < Min_Ring) || (SetPoint > Max_Ring) || (Max_Ring < Min_Ring)) {
        return -1;
    }
    return ((SetPoint * 1.0 - Min_Ring * 1.0) / (Max_Ring * 1.0 - Min_Ring * 1.0));
}
