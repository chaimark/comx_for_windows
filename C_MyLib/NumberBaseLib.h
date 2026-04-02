/*
XXXX_XXX <==> 宏定义
Xxxx_Xxxx <==> 全局变量, 对象名
_Xxxx_Xxx <==> 类名
XxxxXxxxx <==> 局部变量
xxxxXxxx() <==> 函数
xxxxx_xxxx <==> typedef(数据类型)
*/
#ifndef NUMBERBASELIC_H
#define NUMBERBASELIC_H

#include "StrLib.h"
#include <stdint.h>
// 将 uint8_t 数组 转换成 Double Or Float
extern double BuffToFloatOrDouble(strnew OutBuff, bool IsDouble);

// 将 Double Or Float 存到 char 数组 中
extern void DoubleOrFloatToBuff(strnew OutBuff, double Number, bool IsDouble);

// 将数组串转字符串0x01 0x02 ==> 0x31 0x32  （可以原地转换, 需要注意数组串的长度与字符串的长度, 不是整个数组的长度）
extern void numberArrayToStrArray(char StrArray[], char NumberArray[], int ArrayMinLen);

// 将字符串转数组串 0x31 0x32 ==> 0x01 0x02 （可以原地转换, 需要注意数组串的长度与字符串的长度, 不是整个数组的长度）
extern void strArrayToNumberArray(char NumberArray[], char StrArray[], int ArrayMinLen);

// 任意进制互转
extern uint64_t anyBaseToAnyBase(uint64_t Number, int IntputBase, int OutputBase);

// 任意进制数 转 任意进制数组 返回长度 Dex(56) ==> 0x05 0x06 （注意：OutArray 的长度需要大于 Number 的长度,
// 不支持原地转换）
extern int anyBaseNumberToAnyBaseArray(uint64_t Number, int IntputBase, int OutputBase, char OutArray[],
                                       int ArrayMaxLen);

// 任意进制数组 转 任意进制数 string:12345600 ==> 12345600
extern int64_t anyBaseArrayToAnyBaseNumber(char IntArray[], int NumStrNowLen, int IntputBase, int OutputBase);

// 单字节数组 转 双字节数组 0x23 --> 0x02 0x03 （原地转换会出现覆盖, 所以不支持原地, 且需要两个 strnew）
extern int shortChStrToDoubleChStr(strnew inputArray, strnew OutputArray);

// 双字节数组 转 单字节数组 0x02 0x03 --> 0x23 （长度足够, 支持原地转换, 需要两个 strnew）
extern int doubleChStrToShortChStr(strnew inputArray, strnew OutputArray);

// 字符串转 任意进制数
extern int doneAsciiStrToAnyBaseNumberData(char AscArray[], int OutputBase);

// 任意进制数 转 字符串
extern int doneBaseNumberDataToAsciiStr(char AscArray[], int ArrayMaxLen, int NumberData, int IntputBase);

// 字符串 转 double
extern double doneAsciiToDouble(char AscArray[]);

// double 转 字符串
extern void doneDoubleToAscii(char AscArray[], const char From[], double InputData);

// 读取某位 返回对应位的 bool 值
extern bool readDataBit(uint64_t InputNumber, int8_t BitNumber);

// 设置某位 返回是否设置成功
extern uint64_t setDataBit(uint64_t InputNumber, int8_t BitNumber, bool Value);

// 外部接口
extern int HEX2ToASCII(char* hex, int hex_len, char* asc, int asc_len);
extern int ASCIIToHEX2(char* asc, int asc_len, char* hex, int hex_len);

extern uint16_t U8_Connect_U8(uint8_t H_Part, uint8_t L_Part);

// 该宏可以恢复原数组, 如果不需要可直接调用原函数： ASCIIToHEX2
#define AsciiToHex(asc, asc_len, hex, hex_len)                                                                         \
    ASCIIToHEX2(asc, asc_len, hex, hex_len);                                                                           \
    numberArrayToStrArray(asc, asc, asc_len)
#endif

// 获取某个值在某段区间类所在点的百分比
extern float getPartOfSetPointOnRing(uint32_t SetPoint, uint32_t Min_Ring, uint32_t Max_Ring);
