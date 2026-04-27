#include "NumberBaseLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double buffToFloatOrDouble(strnew OutBuff, bool IsDouble) {
    union {
        double TempDouble;
        float TempFloat;
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
void doubleOrFloatToBuff(strnew OutBuff, double Number, bool IsDouble) {
    uint8_t BuffLen = 0;
    union {
        double TempDouble;
        float TempFloat;
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
// hex(0x1f)    //表示 hex(0x1f) = 31
*/

// 将数组串转字符串 0x01 0x02 ==> 0x31 0x32(需要注意: NeedLen 是 NumberArray 中需要转换的长度)
void numberArrayToStrArray(strnew StrArray, const strnew NumberArray, int NeedLen) {
    if (StrArray.MaxLen < NeedLen) {
        return;
    }
    int Addr = 0;
    do {
        if (NumberArray.Name._char[Addr] < 10) {
            StrArray.Name._char[Addr] = NumberArray.Name._char[Addr] + '0';
        } else if ((NumberArray.Name._char[Addr] >= 0x0A) && (NumberArray.Name._char[Addr] <= 0x0F)) {
            StrArray.Name._char[Addr] = NumberArray.Name._char[Addr] + 0x37;
        }
        Addr++;
    } while (Addr < NeedLen);
}
// 将字符串转数组串 0x31 0x32 ==> 0x01 0x02(需要注意: NeedLen 是 StrArray 中需要转换的长度)
void strArrayToNumberArray(strnew NumberArray, const strnew StrArray, int NeedLen) {
    if (NumberArray.MaxLen < NeedLen) {
        return;
    }
    int Addr = 0;
    do {
        if ((StrArray.Name._char[Addr] >= '0') && (StrArray.Name._char[Addr] <= '9')) {
            NumberArray.Name._char[Addr] = StrArray.Name._char[Addr] - '0';
        } else if ((StrArray.Name._char[Addr] >= 'A') && (StrArray.Name._char[Addr] <= 'Z')) {
            NumberArray.Name._char[Addr] = StrArray.Name._char[Addr] - 0x37;
        } else if ((StrArray.Name._char[Addr] >= 'a') && (StrArray.Name._char[Addr] <= 'z')) {
            NumberArray.Name._char[Addr] = StrArray.Name._char[Addr] - 0x57;
        }
        Addr++;
    } while (Addr < NeedLen);
}
// 任意进制互转
uint64_t anyBaseToAnyBase(uint64_t Number, int IntputBase, int OutputBase) {
    uint64_t ResNumber = Number;
    uint8_t TempArray[16] = {0}; // uint64_t也只有16位数 0x0000000000000001

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
static uint64_t anyBaseNumberToSameArray(strnew Output, uint64_t InputNumber) {
    int8_t Add_i = 0;
    uint8_t TempArray[16] = {0}; // uint64_t也只有16位数 0x0000000000000001
    uint64_t OutLen = 0;

    // 将数据转为字符串
    for (Add_i = 0; (Add_i < 16) && (Add_i < Output.MaxLen); Add_i++) {
        TempArray[Add_i] = InputNumber & 0x000000000000000F; // 先存低位
        InputNumber = InputNumber >> 4;
    }
    // 计算长度
    OutLen = ((Output.MaxLen > 16) ? 16 : Output.MaxLen);
    Add_i = ((Output.MaxLen > 16) ? 15 : Output.MaxLen - 1);
    while ((TempArray[Add_i] == 0) && ((Add_i >= 0) && (Add_i < 16))) {
        OutLen--;
        Add_i--;
    }
    if (OutLen != 0) {
        swapStr((char*)TempArray, OutLen);
        for (Add_i = 0; (Add_i < 16) && ((uint64_t)Add_i < OutLen); Add_i++) {
            Output.Name._uint8_t[Add_i] = TempArray[Add_i];
        }
    }
    return OutLen;
}
// 任意进制数组 转 对应进制数 返回数 12345600 ==> 654321
static uint64_t anyArrayToSameBaseNumber(uint8_t* InputStr, int StrSize) {
    uint8_t TempArray[16] = {0}; // uint64_t也只有16位数 0x0000000000000001
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
// 任意进制数 转 任意进制数组
int anyBaseNumberToAnyBaseArray(uint64_t Number, int IntputBase, int OutputBase, strnew OutArray) {
    uint64_t TempNum = anyBaseToAnyBase(Number, IntputBase, OutputBase);
    char TempArray[16] = {0};
    int ArrayLen = anyBaseNumberToSameArray(NEW_NAME(TempArray), TempNum);
    if (OutArray.MaxLen < ArrayLen) {
        return 0;
    }
    memcpy(OutArray.Name._char, TempArray, ArrayLen);
    return ArrayLen;
}

// 任意进制数组 转 任意进制数 Array:12345600 ==> 12345600
int64_t anyBaseArrayToAnyBaseNumber(strnew IntArray, int IntputBase, int OutputBase) {
    swapStr(IntArray.Name._char, IntArray.MaxLen); // 先将数组从从大端模式改为小端
    int64_t TempNum = anyArrayToSameBaseNumber((uint8_t*)IntArray.Name._char, IntArray.MaxLen);
    TempNum = anyBaseToAnyBase(TempNum, IntputBase, OutputBase);
    return TempNum;
}

// 单字节数组 转 双字节数组 0x23 --> 0x02 0x03
int shortChStrToDoubleChStr(const strnew inputArray, strnew OutputArray) {
    if (&(inputArray.Name._char) != &(OutputArray.Name._char)) {
        int ResLen = 0;
        char TempChar[2] = {0};
        if (OutputArray.MaxLen < (inputArray.MaxLen * 2)) {
            return 0;
        }
        for (int i = 0; i < inputArray.MaxLen; i++) {
            memset(TempChar, 0, 2);
            uint8_t InputNum = inputArray.Name._char[i];
            if (anyBaseNumberToSameArray(NEW_NAME(TempChar), InputNum) == 1) {
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
// 双字节数组 转 单字节数组 0x02 0x03 --> 0x23
int doubleChStrToShortChStr(const strnew inputArray, strnew OutputArray) {
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
int doneAsciiStrToAnyBaseNumberData(const char AscArray[], int OutputBase) {
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
    if (NumStrNowLen > strlen(AscArray) || NumStrNowLen == 0) {
        return 0;
    }
    strnew_malloc(NumStr, NumStrNowLen);
    newstrobj(String_temp, 1);
    String_temp.Name._cschar = AscArray;
    String_temp.MaxLen = strlen(AscArray);
    strArrayToNumberArray(NumStr, (const strnew)String_temp, NumStrNowLen); // 去掉 0x30
    int NumTemp = anyBaseArrayToAnyBaseNumber(NumStr, 10, OutputBase);      // 组合成 任意进制数
    return ((sign == 1) ? NumTemp : -NumTemp);
}
// 任意进制数 转 字符串
int doneBaseNumberDataToAsciiStr(strnew AscArray, int NumberData, int IntputBase) {
    uint64_t TempNum = anyBaseToAnyBase(NumberData, IntputBase, 10);
    strnew_malloc(TempArr, 16);
    int AscArrayLen = anyBaseNumberToSameArray(TempArr, TempNum); // 10进制 转对应数组
    numberArrayToStrArray(AscArray, TempArr, AscArrayLen);        // 数组串 转 字符串
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

// 设置某位 返回设置值
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

// 外用接口(不支持原地转换)
int HEX2ToASCII(const strnew hex, strnew asc) {
    if (hex.MaxLen * 2 > asc.MaxLen) { // asc_len 太小, 不够
        return 0;
    }
    // HEX TO STR-------------------
    // 单字节数组 转 双字节数组 0x23 --> 0x02 0x03
    int TempLen = 0;
    if ((TempLen = shortChStrToDoubleChStr(hex, asc)) == 0) {
        return 0;
    }
    strnew_malloc(NumArr, TempLen);
    memcpy(NumArr.Name._char, asc.Name._char, TempLen);
    // 将数组串转字符串
    numberArrayToStrArray(asc, NumArr, TempLen);
    asc.Name._char[2 * hex.MaxLen] = '\0';
    return TempLen;
    // HEX TO STR-------------------
}

// 外用接口(不支持原地转换)
int ASCIIToHEX2(const strnew asc, strnew hex) {
    int strLen = strlen(asc.Name._char);
    if ((strLen > asc.MaxLen) || (hex.MaxLen < strLen / 2) || strlen(asc.Name._char) == 0) { // hex_len 太小, 不够
        return 0;
    }
    // STR TO HEX-------------------
    strnew_malloc(temp_hex, strLen);
    strArrayToNumberArray(temp_hex, asc, temp_hex.MaxLen); // 将字符串转数组串
    int Len = doubleChStrToShortChStr(temp_hex, hex);      // 双字节数组 转 单字节数组 0x02 0x03 --> 0x23
    return Len;
    // STR TO HEX-------------------
}

// 两个 U8 组合成 U16
uint16_t U8_Connect_U8(uint8_t H_Part_Input, uint8_t L_Part_Input) {
    uint16_t H_Part = H_Part_Input;
    uint16_t L_Part = L_Part_Input;
    return (((H_Part & 0x00FF) << 8) | (L_Part & 0x00FF));
}

// 获取某个值在某段区间类所在点的百分比
float getPartOfSetPointOnRing(uint32_t SetPoint, uint32_t Min_Ring, uint32_t Max_Ring) {
    if ((SetPoint < Min_Ring) || (SetPoint > Max_Ring) || (Max_Ring < Min_Ring)) {
        return -1;
    }
    return ((SetPoint * 1.0 - Min_Ring * 1.0) / (Max_Ring * 1.0 - Min_Ring * 1.0));
}

// 给定一个 ip 字符串 192.168.1.1 ==> [c0] [a8] [01] [01]
void iPstrToHexArray(strnew IpHex, const char* Ipstr) { // IP字符串转16进制
    char IP_String[] = {"255.255.255.255.."};
    memset(IP_String, 0, strlen("255.255.255.255.."));
    memcpy(IP_String, Ipstr, strlen(Ipstr));
    catString(IP_String, ".", strlen("255.255.255.255.."), 1); // 字符串拼接
    char Str[4] = {0};
    char* P_piont = NULL;
    char* Head = IP_String;
    int temp = 0;
    for (int i = 0; i < 4; i++) {
        memset(Str, 0, 3);
        if ((P_piont = strchr(Head, '.')) != NULL) {
            *P_piont = '\0';
            memcpy(Str, Head, strlen(Head));
            *P_piont = '.';
            Head = P_piont + 1;
            temp = doneAsciiStrToAnyBaseNumberData(Str, 16); // 字符串转任意进制数
            IpHex.Name._char[i] = (unsigned char)temp;
        }
    }
}
