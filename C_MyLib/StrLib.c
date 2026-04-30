#include "StrLib.h"
#include <stdint.h>
#ifdef _Alignas
Type_T _InitType(void* var, const char* type) {
    Type_T Temp;
    Temp.var = var;
    Temp.type = type;
    return Temp;
}
#endif
/*-----------------------------------函数说明----------------------------------*/
int catString(char* OutStr, const char* IntStr, int MaxSize, int IntSize);
bool copyString(char* OutStr, const char* IntStr, int MaxSize, int IntSize);
char* myStrstr(char* MotherStr, char* SonStr, int MotherMaxSize);
char* myStrstrCont(char* MotherStr, char* SonStr, int MotherMaxSize, int ContNum);
void swapChr(char* a, char* b);
void swapStr(char* IntputStr, int StrLen);
char swapLowHight_Of_Char(char InputCh);
bool moveDataOnBuff(strnew IntptBuff, int ShiftLen, bool IsLeft);
void stringSlice(strnew OutStr, strnew Mather, int start, int end);
uint32_t getTimeNumber_UTCByRTCTime(strnew RTCTime_String);
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*-----------------------------------建立数组对象----------------------------------*/
void cleanStrnew(strnew This) {
    // 释放私有数据的内存
    if (This.Name._char) {
        free(This.Name._char);
        This.Name._char = NULL;
    }
}
// 新建数组对象 (返回一个结构体数据)
strnew New_Str_Obj(void* Master, int SizeNum, int SizeType) {
    strnew newArray = {0};
    newArray.SizeType = SizeType;
    newArray.MaxLen = SizeNum;
    newArray.Name._void = Master;
    return newArray;
}

/*-----------------------------------函数定义----------------------------------*/
// 追加字符
int catString(char* OutStr, const char* IntStr, int MaxSize, int IntSize) {
    int AddrNow = 0;
    // 找出"\0"
    while ((OutStr[AddrNow] != '\0') && (AddrNow < MaxSize)) {
        if (AddrNow > MaxSize) {
            return -1;
        }
        AddrNow++;
    }
    if (MaxSize >= AddrNow + IntSize) {
        // 开始追加
        int run = 0;
        for (run = 0; run < IntSize; run++) {
            OutStr[AddrNow++] = IntStr[run];
            if (IntStr[run] == '\0') {
                break;
            }
        }
        if (run >= IntSize) {
            if (MaxSize == AddrNow) {
                OutStr[MaxSize - 1] = '\0'; // could make conditional but not neccesary.
            } else {
                OutStr[AddrNow + 0] = '\0'; // could make conditional but not neccesary.
            }
            return AddrNow;
        }
        return AddrNow;
    } else {
        return -1;
    }
}
// 复制字符串
bool copyString(char* OutStr, const char* IntStr, int MaxSize, int IntSize) {
    int run = 0;
    if (MaxSize >= IntSize) {
        for (run = 0; run < IntSize; run++) { // last reserved for'\0'
            OutStr[run] = IntStr[run];
            if (OutStr[run] == '\0') {
                break;
            }
        }
        if (run >= IntSize) {
            if (MaxSize == IntSize) {
                OutStr[run - 1] = '\0'; // could make conditional but not neccesary.
            } else {
                OutStr[run + 0] = '\0'; // could make conditional but not neccesary.
            }
        }
        return true;
    } else {
        return false;
    }
}
// 查找子字符串
char* myStrstr(char* MotherStr, char* SonStr, int MotherMaxSize) {
    char* p_star = NULL;
    int TempAdd = 0;
    int TempStart = 0;
    do {
        for (TempAdd = TempStart; TempAdd < MotherMaxSize; TempAdd++) {
            if (MotherStr[TempAdd] != '\0') {
                TempStart = TempAdd; // 更新记录首地址位
                break;
            }
        }
        if (TempAdd >= MotherMaxSize) {
            return NULL;
        }
        if ((p_star = strstr(&MotherStr[TempStart], SonStr)) != NULL) {
            return p_star; // 找到了
        } else {
            for (TempAdd = TempStart; TempAdd < MotherMaxSize; TempAdd++) {
                if (MotherStr[TempAdd] == '\0') {
                    TempStart = TempAdd; // 没找到。记录下一个首地址
                    break;
                }
            }
            if (TempAdd >= MotherMaxSize) { // 数组遍历结束, 没有找到子串
                return NULL;
            }
        }
    } while (TempStart < MotherMaxSize);
    return NULL;
}

// 查找第N个子字符串
char* myStrstrCont(char* MotherStr, char* SonStr, int MotherMaxSize, int ContNum) {
    char* p_star = NULL;
    char* p_end = (MotherStr + MotherMaxSize);
    for (int Task_i = 0; Task_i < ContNum; Task_i++) {
        p_star = NULL;
        if ((p_star = myStrstr(MotherStr, SonStr, MotherMaxSize)) != NULL) {
            MotherStr = p_star + 1; // 地址偏移, 准备寻找第下一个
            MotherMaxSize = p_end - MotherStr;
        } else
            return NULL; // 没有这个字串
    }
    return p_star;
}

//  交换两个字符
void swapChr(char* a, char* b) {
    char temp = *a;
    *a = *b;
    *b = temp;
}

// 实现字符串逆序的函数
void swapStr(char* InputStr, int StrLen) {
    int start = 0;
    int end = StrLen - 1;

    // 循环交换字符串首尾字符, 直到中间
    while (start < end) {
        swapChr(&InputStr[start], &InputStr[end]);
        start++;
        end--;
    }
}

// 实现单字节翻转的函数
char swapLowHight_Of_Char(char InputCh) {
    char DataH = (InputCh & 0xF0) >> 4;
    char DataL = (InputCh & 0x0F) << 4;

    return (DataL | DataH);
}

// 实现将数组左移或右移动某个长度的函数
bool moveDataOnBuff(strnew IntptBuff, int ShiftLen, bool IsLeft) {
    if (ShiftLen > IntptBuff.MaxLen) {
        return false;
    }
    if (IsLeft) {
        for (int i = 0; i < IntptBuff.MaxLen - ShiftLen; i++) {
            IntptBuff.Name._char[i] = IntptBuff.Name._char[i + ShiftLen];
        }
        for (int i = 0; i < ShiftLen; i++) {
            IntptBuff.Name._char[(IntptBuff.MaxLen - ShiftLen) + i] = 0;
        }
    } else {
        for (int i = IntptBuff.MaxLen - 1; i >= ShiftLen; i--) {
            IntptBuff.Name._char[i] = IntptBuff.Name._char[i - ShiftLen];
        }
        for (int i = 0; i < ShiftLen; i++) {
            IntptBuff.Name._char[i] = 0;
        }
    }
    return true;
}
void stringSlice(strnew OutStr, strnew Mather, int start, int end) {
    int NeedLen = (end >= start ? (end - start) : (start - end));
    if ((NeedLen + start) > Mather.MaxLen) {
        return;
    }
    if (NeedLen > OutStr.MaxLen) {
        return;
    }
    memcpy(OutStr.Name._char, &Mather.Name._char[start], NeedLen);
}
