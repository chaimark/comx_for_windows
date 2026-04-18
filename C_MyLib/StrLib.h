
#ifndef STRLIB_H // 如果 STRLIB_H 未定义
#define STRLIB_H // 定义 STRLIB_H

#include "stdint.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/*-----------------------------------定义数组类----------------------------------*/
#define This (*_THIS_MY_)
#define ARR_SIZE(ArrName) (sizeof(ArrName) / sizeof(ArrName[0])) // 计算数组元素个数
/*
数组类, 包含数组指针和长度
需要注意的是
函数需要返回该类型时
该指针指向的数组需要定义在全局空间
该类型容易误认为是实体对象, 需要注意
可以定义一个全局空间, 用于暂时存放 strnew 对象的数据
或需要使用时直接定义数据, 并作为参数传入
*/
// 类定义
typedef struct New_Arr {
    union _newclass {
        int8_t*        _int8_t;   //  8 位
        int16_t*       _int16_t;  // 16 位
        int32_t*       _int32_t;  // 32 位
        int64_t*       _int64_t;  // 64 位
        uint8_t*       _uint8_t;  // 无符号整  8 位
        uint16_t*      _uint16_t; // 无符号整 16 位
        uint32_t*      _uint32_t; // 无符号整 32 位
        uint64_t*      _uint64_t; // 无符号整 64 位
        void*          _void;     // 无定义形指针
        const char*    _cschar;   // 字符型指针只读数据
        char*          _char;     // 字符型指针
        int*           _int;      // 整型指针
        float*         _float;    // 单精度浮点型指针
        double*        _double;   // 双精度浮点型指针
        unsigned int*  _uint;     // 无符号整型指针
        unsigned char* _uchar;    // 无符号字符型指针
    } Name;
    int MaxLen;
    int SizeType;
} strnew;

// 建立对象
extern strnew New_Str_Obj(void* Master, int SizeNum, int SizeType); // 建立对象的函数
#define NEW_NAME(ArrName) New_Str_Obj(ArrName, ARR_SIZE(ArrName), sizeof(ArrName[0]))
#define newString(name, Len)                                                                                           \
    char   Str##name[Len] = {0};                                                                                       \
    strnew name = NEW_NAME(Str##name);

#define newstrobj(name, TypeSize)                                                                                      \
    strnew name = {0};                                                                                                 \
    name.SizeType = 1;

// 清理对象
void clean_strnew(strnew This);
// 注意：NameStr.Name._char 是由 malloc 申请的空间
// 不能直接使用 NameStr.Name._char = "aaa"，会导致 free 清理失败
#define strnew_malloc(NameStr, Len)                                                                                    \
    __attribute__((cleanup(clean_strnew))) strnew NameStr = {0};                                                       \
    NameStr.SizeType = 1;                                                                                              \
    NameStr.Name._char = (char*)malloc(Len);                                                                           \
    NameStr.MaxLen = Len

/*-----------------------------------外部接口----------------------------------*/
extern int   catString(char* OutStr, const char* IntStr, int MaxSize, int IntSize);
extern bool  copyString(char* OutStr, const char* IntStr, int MaxSize, int IntSize);
extern char* myStrstr(char* MotherStr, char* SonStr, int MotherMaxSize);
extern char* myStrstrCont(char* MotherStr, char* SonStr, int MotherMaxSize, int ContNum);
extern void  swapChr(char* a, char* b);
extern void  swapStr(char* IntputStr, int StrLen);
extern char  swapLowHight_Of_Char(char InputCh);
extern bool  MoveDataOnBuff(strnew IntptBuff, int ShiftLen, bool IsLeft);
extern void  StringSlice(strnew OutStr, strnew Mather, int start, int end);
///////////////////////////////////////////////////////////
extern int      isLeapYear(uint32_t year);
extern uint32_t get_timestamp(uint32_t NowYear, uint32_t NowMonth, uint32_t NowDay, uint32_t NowHour,
                              uint32_t NowMinute, uint32_t NowSecond);
extern uint32_t getTimeNumber_UTCByRTCTime(strnew RTCTime_String);
typedef struct _TimeStuClass {
    uint32_t year;   /** 年 */
    uint32_t month;  /** 月 */
    uint32_t day;    /** 日 */
    uint32_t week;   /** 周 */
    uint32_t hour;   /** 时 */
    uint32_t minute; /** 分 */
    uint32_t second; /** 秒 */
} TimeStuClass;
extern TimeStuClass TimestampToRTCData(uint32_t timestamp);
///////////////////////////////////////////////////////////

#ifdef _Alignas
#define GET_TYPE(var)                                                                                                  \
    (_Generic((var), \
    int: "int", \
    unsigned int: "unsigned int", \
    char: "char", \
    unsigned char: "unsigned char", \
    double: "double", \
    float: "float", \
    char *: "char *", \
    unsigned char *: "unsigned char *", \
    default: "unknown"))

typedef struct _Type_T {
    void*       var;
    const char* type;
} Type_T;

extern Type_T _InitType(void* var, const char* type);
#define newType_X(TypeName, var) Type_T TypeName = _InitType(var, GET_TYPE(var))

#define newType1(var) _InitType(var, GET_TYPE(var))
#define newType2(Name, var) newType_X(Name, var)

#define GET_TYPE_MACRO(_1, _2, NAME, ...) NAME
#define newType(...) GET_TYPE_MACRO(__VA_ARGS__, newType2, newType1)(__VA_ARGS__)
#endif

#endif
