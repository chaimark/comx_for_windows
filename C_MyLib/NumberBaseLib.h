/*
XXXX_XXX <==> 宏定义
Xxxx_Xxxx <==> 全局变量, 对象名
_Xxxx_Xxx <==> 类名
XxxxXxxxx <==> 局部变量
xxxxXxxx() <==> 函数
xxxxx_xxxx <==> typedef(数据类型)
*/
#ifndef __NUMBERBASELIC_H__
#define __NUMBERBASELIC_H__

#include "StrLib.h"
#include <stdint.h>
// 将 uint8_t 数组 转换成 Double Or Float
extern double buffToFloatOrDouble(strnew OutBuff, bool IsDouble);

// 将 Double Or Float 存到 char 数组 中
extern void doubleOrFloatToBuff(strnew OutBuff, double Number, bool IsDouble);

// 将数组串转字符串 0x01 0x02 ==> 0x31 0x32(需要注意: NeedLen 是 NumberArray 中需要转换的长度)
extern void numberArrayToStrArray(strnew StrArray, const strnew NumberArray, int NeedLen);

// 将字符串转数组串 0x31 0x32 ==> 0x01 0x02(需要注意: NeedLen 是 StrArray 中需要转换的长度)
extern void strArrayToNumberArray(strnew NumberArray, const strnew StrArray, int NeedLen);

// 任意进制互转
extern uint64_t anyBaseToAnyBase(uint64_t Number, int IntputBase, int OutputBase);

// 任意进制数 转 任意进制数组 返回长度 Dex(56) ==> 0x05 0x06 (注意：OutArray 的长度需要大于 Number 的长度)
extern int anyBaseNumberToAnyBaseArray(uint64_t Number, int IntputBase, int OutputBase, strnew OutArray);

// 注意 strnew 数组必须沾满，任意进制数组 转 任意进制数 string:12345600 ==> 12345600
extern int64_t anyBaseArrayToAnyBaseNumber(strnew IntArray, int IntputBase, int OutputBase);

// 单字节数组 转 双字节数组 0x23 --> 0x02 0x03
extern int shortChStrToDoubleChStr(const strnew inputArray, strnew OutputArray);

// 双字节数组 转 单字节数组 0x02 0x03 --> 0x23
extern int doubleChStrToShortChStr(const strnew inputArray, strnew OutputArray);

// 字符串转 任意进制数
extern int doneAsciiStrToAnyBaseNumberData(const char AscArray[], int OutputBase);

// 任意进制数 转 字符串
extern int doneBaseNumberDataToAsciiStr(strnew AscArray, int NumberData, int IntputBase);

// 字符串 转 double
extern double doneAsciiToDouble(char AscArray[]);

// double 转 字符串
extern void doneDoubleToAscii(char AscArray[], const char From[], double InputData);

// 读取某位 返回对应位的 bool 值
extern bool readDataBit(uint64_t InputNumber, int8_t BitNumber);

// 设置某位 返回设置值
extern uint64_t setDataBit(uint64_t InputNumber, int8_t BitNumber, bool Value);

// 外部接口(不支持原地转换)
extern int HEX2ToASCII(const strnew hex, strnew asc);
extern int ASCIIToHEX2(const strnew asc, strnew hex);

// 两个 U8 组合成 U16
extern uint16_t U8_Connect_U8(uint8_t H_Part, uint8_t L_Part);

// 获取某个值在某段区间类所在点的百分比
extern float getPartOfSetPointOnRing(uint32_t SetPoint, uint32_t Min_Ring, uint32_t Max_Ring);

// 给定一个 ip 字符串 192.168.1.1 ==> [c0] [a8] [01] [01]
extern void iPstrToHexArray(strnew IpHex, const char* Ipstr);

#endif
