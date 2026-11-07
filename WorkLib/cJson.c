#include "NumberBaseLib.h"
#include "cJson.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//==========================================================================================//
//==========================================================================================//
// #define IsOpenFloatHelp_Ability
#ifdef IsOpenFloatHelp_Ability
static char getNowType(const char *NowAddr, char *UserFromNow) {
    if ((*NowAddr != '%') && (NowAddr + 1 != NULL)) {
        return 0;
    }
    int Len = 0;
    while (((NowAddr + 1) != NULL) && (*(NowAddr + 1) != '%')) {
        UserFromNow[Len++] = *(NowAddr++);
        switch (*NowAddr) {
        case 'l':
            UserFromNow[Len++] = *(NowAddr + 0);
            UserFromNow[Len++] = *(NowAddr + 1);
            NowAddr++;
            return ((*(NowAddr) == 'f' ? 'F' : 'D'));
        case 'd':
            UserFromNow[Len++] = *(NowAddr++);
            return 'd';
        case 'o':
            UserFromNow[Len++] = *(NowAddr++);
            return 'o';
        case 'x':
            UserFromNow[Len++] = *(NowAddr++);
            return 'x';
        case 'u':
            UserFromNow[Len++] = *(NowAddr++);
            return 'u';
        case 'f':
            UserFromNow[Len++] = *(NowAddr++);
            return 'f';
        case 'c':
            UserFromNow[Len++] = *(NowAddr++);
            return 'c';
        case 's':
            UserFromNow[Len++] = *(NowAddr++);
            return 's';
        }
    }
    return 0;
}

static bool getFromTypeCheckDoubleOrFloat(strnew FromStr) {
    // 没有 % 退出
    if (strchr(FromStr.Name._char, '%') == NULL) {
        return false;
    }
    const char *NowAddr = FromStr.Name._char;
    do {
        if (strchr(NowAddr, '%') == NULL) {
            break;
        }
        while (((*NowAddr) != '%') && ((NowAddr + 1) != NULL)) {
            NowAddr++; // 不是 %
            continue;
        }
        if ((*(NowAddr + 1) == '%') && ((NowAddr + 1) != NULL)) { // 是否是 %%
            NowAddr++;
            continue;
        }
        char UserFromNow[10] = {0};
        // 找到格式字符串里的第一个 %
        if ((getNowType(NowAddr, UserFromNow) == 'f') || (getNowType(NowAddr, UserFromNow) == 'F')) {
            return true;
        }
        NowAddr += strlen(UserFromNow); // 准备找下一个 %
    } while ((NowAddr + 1) != NULL);
    return false;
}
#endif

static bool isNeedBySignDivde(strnew InputString, int Addr_Over) {
    int ReFlag = 1; // 先预留, 并寻找最后一个字符 '}'
    for (int i = Addr_Over; i > 0; i--) {
        if (InputString.Name._char[i] == ' ' || InputString.Name._char[i] == '\0') {
            continue;
        }
        if (InputString.Name._char[i] == '\n' || InputString.Name._char[i] == '\r') {
            continue;
        }
        if ((ReFlag > 0) && ((InputString.Name._char[i] == '}') || (InputString.Name._char[i] == ']'))) {
            ReFlag--;
            continue;
        }
        if (ReFlag < 1) {
            if ((InputString.Name._char[i] != '{') && (InputString.Name._char[i] != '[')) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

void addJsonItemData(strnew JsonStringSpace, const char *FromStr, ...) {
    char KeyName[200] = {0};
    // 查找 :
    char *Addr_OverName = NULL;
    Addr_OverName = strchr(FromStr, ':');
    const char *Addr_Start = strchr(FromStr, ',');
    if ((Addr_Start != NULL) && (Addr_OverName != NULL) && (Addr_Start < Addr_OverName)) {
        Addr_Start++;
        catString(KeyName, FromStr, 100, (Addr_Start - FromStr));
    } else {
        Addr_Start = FromStr;
    }

    if (Addr_OverName != NULL) {
        catString(KeyName, "\"", 100, 1);
        catString(KeyName, Addr_Start, 100, (Addr_OverName - Addr_Start));
        catString(KeyName, "\"", 100, 1);
        catString(KeyName, Addr_OverName, 100, strlen(Addr_OverName));
    } else {
        memcpy(KeyName, Addr_Start, (strlen(Addr_Start) < 200 ? strlen(Addr_Start) : 200));
    }

    // 获取当前字符串的长度
    int Addr_Over = strlen(JsonStringSpace.Name._char);
    // 初始化可变参数列表
    va_list args;
    va_start(args, FromStr);
    if (Addr_Over > 2) {
        if (isNeedBySignDivde(JsonStringSpace, Addr_Over) == false) {
            JsonStringSpace.Name._char[Addr_Over--] = '\0';
        } else {
            JsonStringSpace.Name._char[Addr_Over - 1] = ',';
        }
    } else if (Addr_Over == 2) {
        JsonStringSpace.Name._char[Addr_Over--] = '\0';
    }
    vsprintf(&JsonStringSpace.Name._char[Addr_Over], KeyName, args);
    // 结束可变参数处理
    va_end(args);
#ifdef IsOpenFloatHelp_Ability
    if (getFromTypeCheckDoubleOrFloat(NEW_NAME(KeyName)) || getFromTypeCheckDoubleOrFloat(NEW_NAME(KeyName))) {
        int NowStrLen = Addr_Over;
        while (!((JsonStringSpace.Name._char[NowStrLen] == '\0') && (JsonStringSpace.Name._char[NowStrLen + 1] == '\0') && (JsonStringSpace.Name._char[NowStrLen + 2] == '\0'))) {
            NowStrLen++;
        }
        while (Addr_Over < NowStrLen) {
            if (JsonStringSpace.Name._char[Addr_Over] == 0) {
                JsonStringSpace.Name._char[Addr_Over] = '.';
            }
            Addr_Over++;
        }
    }
#endif
    if (JsonStringSpace.Name._char[0] == '{') {
        catString(JsonStringSpace.Name._char, "}", JsonStringSpace.MaxLen, 1);
    } else {
        catString(JsonStringSpace.Name._char, "]", JsonStringSpace.MaxLen, 1);
    }
}

//==========================================================================================//
//==========================================================================================//
#define MAX_STACK_SIZE 100 // 定义栈的最大容量
typedef struct {
    char data[MAX_STACK_SIZE];
    int top;
} Stack;
// 初始化栈
static void initStack(Stack *s) {
    s->top = -1;
}
// 判断栈是否为空
static bool isEmpty(Stack *s) {
    return s->top == -1;
}
// 入栈
static void push(Stack *s, char ch) {
    if (s->top < MAX_STACK_SIZE - 1) {
        s->data[++(s->top)] = ch;
    }
}
// 出栈
static char pop(Stack *s) {
    if (!isEmpty(s)) {
        return s->data[(s->top)--];
    }
    return '\0'; // 返回一个空字符
}
//==========================================================================================//
//==========================================================================================//

// 查找双重字符位置
static char *getDoubleChrOnString(char *MotherString, char HeadChr, char EndChr) {
    Stack s;
    initStack(&s);
    char *result = NULL;
    bool InString = false;
    for (int i = 0; (MotherString[i] != '\0'); i++) {
        char Chr = MotherString[i];
        if (InString) { // 字符串内容里的括号和转义不参与匹配
            if (Chr == '\\') {
                if (MotherString[i + 1] == '\0') {
                    break;
                }
                i++; // 跳过被转义的字符
            } else if (Chr == '\"') {
                InString = false;
            }
            continue;
        }
        if (Chr == '\"') {
            InString = true;
            continue;
        }
        if (Chr == HeadChr) {
            push(&s, HeadChr);
        } else if (Chr == EndChr) {
            if (!isEmpty(&s)) {
                pop(&s);
                if (isEmpty(&s)) {
                    result = &MotherString[i];
                    break;
                }
            }
        }
    }
    return result;
}

// 跳过 JSON 字符串, P 指向起始引号, 返回收尾引号后一位
static char *skipJsonString(char *P) {
    P++;
    while (*P != '\0') {
        if (*P == '\\') {
            if (*(P + 1) == '\0') {
                return P + 1;
            }
            P += 2;
        } else if (*P == '\"') {
            return (P + 1);
        } else {
            P++;
        }
    }
    return P;
}
// 跳过对象/数组值, P 指向 '{' 或 '['
static char *skipJsonComposite(char *P) {
    int Depth = 0;
    bool InString = false;
    for (; *P != '\0'; P++) {
        char Chr = *P;
        if (InString) {
            if (Chr == '\\') {
                if (*(P + 1) == '\0') {
                    return (P + 1);
                }
                P++;
            } else if (Chr == '\"') {
                InString = false;
            }
            continue;
        }
        if (Chr == '\"') {
            InString = true;
            continue;
        }
        if ((Chr == '{') || (Chr == '[')) {
            Depth++;
        } else if ((Chr == '}') || (Chr == ']')) {
            Depth--;
            if (Depth <= 0) {
                return (P + 1);
            }
        }
    }
    return P;
}
// 跳过一个值, 返回其后的逗号或 '}' 位置
static char *skipJsonValue(char *P) {
    if (*P == '\"') {
        return skipJsonString(P);
    }
    if ((*P == '{') || (*P == '[')) {
        return skipJsonComposite(P);
    }
    while ((*P != '\0') && (*P != ',') && (*P != '}')) {
        P++;
    }
    return P;
}

// 查找对象根层级中 "Key" 对应的值首地址, 允许冒号前后有空白
// 跳过嵌套对象/数组和字符串内容; 未找到返回 NULL
static char *getJsonValueStart(char *JsonString, char Key[]) {
    size_t KeyLen = strlen(Key);
    char *P = JsonString;
    while ((*P == ' ') || (*P == '\t') || (*P == '\r') || (*P == '\n')) {
        P++;
    }
    if (*P != '{') { // 只处理对象
        return NULL;
    }
    P++;
    while (*P != '\0') {
        while ((*P == ' ') || (*P == '\t') || (*P == '\r') || (*P == '\n') || (*P == ',')) {
            P++;
        }
        if ((*P == '}') || (*P == '\0')) {
            return NULL;
        }
        if (*P != '\"') { // 非标准成员, 跳到下一个分隔符
            while ((*P != '\0') && (*P != ',') && (*P != '}')) {
                P++;
            }
            continue;
        }
        char *KeyStart = P + 1;
        char *AfterKey = skipJsonString(P);
        char *Q = AfterKey;
        while ((*Q == ' ') || (*Q == '\t') || (*Q == '\r') || (*Q == '\n')) {
            Q++;
        }
        if (*Q != ':') { // 不是键值对, 跳过
            P = Q;
            continue;
        }
        Q++;
        while ((*Q == ' ') || (*Q == '\t') || (*Q == '\r') || (*Q == '\n')) {
            Q++;
        }
        size_t KeyTextLen = (size_t)((AfterKey - 1) - KeyStart);
        if ((KeyTextLen == KeyLen) && (memcmp(KeyStart, Key, KeyLen) == 0)) {
            return Q;
        }
        P = skipJsonValue(Q);
    }
    return NULL;
}

//==========================================================================================//
JsonArray newJsonArrayByString(strnew DataInit);
JsonObject newJsonObjectByString(strnew DataInit);
//==========================================================================================//
// 判断数组是否为空: 跳过根 '[', 只允许空白后紧跟 ']' 或结束符
static int Arr_sizeItemNum(struct _JsonArray This) {
    if (This.ItemNum != -1) {
        return This.ItemNum;
    }
    char *P = This.JsonString.Name._char;
    while ((*P != '\0') && (*P != '[')) { // 跳过根前空白
        P++;
    }
    if (*P != '[') {
        return 0;
    }
    P++;
    while ((*P == ' ') || (*P == '\t') || (*P == '\r') || (*P == '\n')) {
        P++;
    }
    if ((*P == ']') || (*P == '\0')) { // 空数组
        return 0;
    }
    int Depth = 0;
    int ItemNum = 1; // 至少有一个元素
    bool InString = false;
    for (; *P != '\0'; P++) {
        char Chr = *P;
        if (InString) { // 字符串内容不参与切分
            if (Chr == '\\') {
                if (*(P + 1) == '\0') {
                    break;
                }
                P++;
            } else if (Chr == '\"') {
                InString = false;
            }
            continue;
        }
        if (Chr == '\"') {
            InString = true;
            continue;
        }
        if ((Chr == '{') || (Chr == '[')) {
            Depth++;
            continue;
        }
        if ((Chr == '}') || (Chr == ']')) {
            if (Depth > 0) {
                Depth--;
                continue;
            }
            if (Chr == ']') { // 根数组结束
                break;
            }
            continue;
        }
        if ((Chr == ',') && (Depth == 0)) { // 只统计顶层逗号
            ItemNum++;
        }
    }
    This.ItemNum = ItemNum;
    return ItemNum;
}
static signed char Arr_isJsonNull(struct _JsonArray This) {
    char *StartP = NULL;
    if ((StartP = strchr(This.JsonString.Name._char, '[')) == NULL) {
        return false;
    }
    StartP++; // 跳过根 '['
    while ((*StartP == ' ') || (*StartP == '\t') || (*StartP == '\r') || (*StartP == '\n')) {
        StartP++;
    }
    return ((*StartP == ']' || *StartP == '\0') ? true : false);
}
static bool Arr_getItemRange(char *JsonString, int ItemNum, char **ItemStart, int *ItemLen);

static void Arr_get(struct _JsonArray This, strnew OutStr, int ItemNum) {
    memset(OutStr.Name._char, 0, OutStr.MaxLen);
    char *StartP = NULL;
    int ItemLen = 0;
    if (Arr_getItemRange(This.JsonString.Name._char, ItemNum, &StartP, &ItemLen) == false) {
        return;
    }
    if (StartP[0] == '\"') { // 字符串元素去掉外层引号, 保留内部转义
        int ContentLen = ItemLen - 2;
        if (ContentLen < 0) {
            ContentLen = 0;
        }
        if (ContentLen >= OutStr.MaxLen) {
            ContentLen = OutStr.MaxLen - 1;
        }
        if (ContentLen > 0) {
            memcpy(OutStr.Name._char, (StartP + 1), (size_t)ContentLen);
        }
        OutStr.Name._char[ContentLen] = '\0';
        return;
    }
    if (ItemLen >= OutStr.MaxLen) { // 输出空间不足时截断
        ItemLen = OutStr.MaxLen - 1;
    }
    if (ItemLen > 0) {
        memcpy(OutStr.Name._char, StartP, (size_t)ItemLen);
    }
    OutStr.Name._char[ItemLen] = '\0';
}
// 定位数组第 ItemNum 个顶层元素的原始区间, 保留元素自身的括号/引号/嵌套结构
// 跳过字符串内容与转义, 只按顶层逗号和根 ']' 切分; 未找到返回 false
static bool Arr_getItemRange(char *JsonString, int ItemNum, char **ItemStart, int *ItemLen) {
    char *P = JsonString;
    while ((*P != '\0') && (*P != '[')) {
        P++;
    }
    if (*P != '[') {
        return false;
    }
    P++;
    while ((*P == ' ') || (*P == '\t') || (*P == '\r') || (*P == '\n')) {
        P++;
    }
    if ((*P == ']') || (*P == '\0')) {
        return false;
    }
    int Depth = 0;
    int Index = 0;
    bool InString = false;
    char *Start = P;
    for (; *P != '\0'; P++) {
        char Chr = *P;
        if (InString) { // 字符串内容不参与切分
            if (Chr == '\\') {
                if (*(P + 1) == '\0') {
                    break;
                }
                P++;
            } else if (Chr == '\"') {
                InString = false;
            }
            continue;
        }
        if (Chr == '\"') {
            InString = true;
            continue;
        }
        if ((Chr == '{') || (Chr == '[')) {
            Depth++;
            continue;
        }
        if ((Chr == '}') || (Chr == ']')) {
            if (Depth > 0) {
                Depth--;
                continue;
            }
            if (Chr == ']') { // 根数组结束
                if (Index == ItemNum) {
                    char *End = P;
                    while ((End > Start) && ((*(End - 1) == ' ') || (*(End - 1) == '\t') || (*(End - 1) == '\r') || (*(End - 1) == '\n'))) {
                        End--;
                    }
                    *ItemStart = Start;
                    *ItemLen = (int)(End - Start);
                    return true;
                }
                return false;
            }
            continue;
        }
        if ((Chr == ',') && (Depth == 0)) {
            if (Index == ItemNum) {
                char *End = P;
                while ((End > Start) && ((*(End - 1) == ' ') || (*(End - 1) == '\t') || (*(End - 1) == '\r') || (*(End - 1) == '\n'))) {
                    End--;
                }
                *ItemStart = Start;
                *ItemLen = (int)(End - Start);
                return true;
            }
            Index++;
            Start = P + 1;
            while ((*Start == ' ') || (*Start == '\t') || (*Start == '\r') || (*Start == '\n')) {
                Start++;
            }
        }
    }
    return false;
}
static void Arr_getArray(struct _JsonArray This, strnew OutStr, int ItemNum) {
    if (OutStr.Name._char == This.JsonString.Name._char) {
        return;
    }
    memset(OutStr.Name._char, 0, OutStr.MaxLen);
    char *StartP = NULL;
    int ItemLen = 0;
    if (Arr_getItemRange(This.JsonString.Name._char, ItemNum, &StartP, &ItemLen) == false) {
        return;
    }
    if (ItemLen >= OutStr.MaxLen) { // 输出空间不足时截断, 保证有结束符
        ItemLen = OutStr.MaxLen - 1;
    }
    if (ItemLen > 0) {
        memcpy(OutStr.Name._char, StartP, (size_t)ItemLen);
    }
    OutStr.Name._char[ItemLen] = '\0';
}
JsonArray newJsonArrayByString(strnew DataInit) {
    JsonArray Temp;
    Temp.JsonString = DataInit;
    Temp.ItemNum = -1;
    Temp.sizeItemNum = Arr_sizeItemNum;
    Temp.isJsonNull = Arr_isJsonNull;
    Temp.get = Arr_get;
    Temp.getArray = Arr_getArray;
    return Temp;
}

//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//

// 统计对象根层级的键值对个数, 跳过嵌套对象/数组与字符串内容
static int Obj_sizeStr(struct _JsonObject This) {
    char *P = This.JsonString.Name._char;
    while ((*P == ' ') || (*P == '\t') || (*P == '\r') || (*P == '\n')) {
        P++;
    }
    if (*P != '{') { // 只处理对象
        return 0;
    }
    P++;
    int Count = 0;
    while (*P != '\0') {
        while ((*P == ' ') || (*P == '\t') || (*P == '\r') || (*P == '\n') || (*P == ',')) {
            P++;
        }
        if ((*P == '}') || (*P == '\0')) {
            break;
        }
        if (*P != '\"') { // 非标准成员, 跳到下一个分隔符
            while ((*P != '\0') && (*P != ',') && (*P != '}')) {
                P++;
            }
            continue;
        }
        char *AfterKey = skipJsonString(P);
        char *Q = AfterKey;
        while ((*Q == ' ') || (*Q == '\t') || (*Q == '\r') || (*Q == '\n')) {
            Q++;
        }
        if (*Q != ':') { // 不是键值对, 跳过
            P = Q;
            continue;
        }
        Q++;
        while ((*Q == ' ') || (*Q == '\t') || (*Q == '\r') || (*Q == '\n')) {
            Q++;
        }
        Count++;
        P = skipJsonValue(Q);
    }
    return Count;
}
static signed char Obj_isJsonNull(struct _JsonObject This, char Key[]) {
    signed char ResOver = -1;
    char *KeyP = getJsonValueStart(This.JsonString.Name._char, Key);
    if (KeyP != NULL) {
        ResOver = false;
        // 只比较前缀, 不再临时改写输入; strncmp 遇到 '\0' 会停止, 不会越界
        if ((strncmp(KeyP, "null", 4) == 0) || (strncmp(KeyP, "NULL", 4) == 0) || (strncmp(KeyP, "Null", 4) == 0)) {
            ResOver = true;
        }
    }
    return ResOver;
}
static int Obj_getInt(struct _JsonObject This, char Key[]) {
    char *KeyP = getJsonValueStart(This.JsonString.Name._char, Key);
    if (KeyP != NULL) {
        return doneAsciiStrToAnyBaseNumberData(KeyP, 16);
        // return atol(KeyP);
    }
    return 0;
}
static double Obj_getDouble(struct _JsonObject This, char Key[]) {
    char *KeyP = getJsonValueStart(This.JsonString.Name._char, Key);
    if (KeyP != NULL) {
        return atof(KeyP);
    }
    return 0.0;
}
static bool Obj_getBool(struct _JsonObject This, char Key[]) {
    bool ResBool = false;
    char *KeyP = getJsonValueStart(This.JsonString.Name._char, Key);
    if (KeyP != NULL) {
        // 只比较前缀, 不再临时改写输入; "false" 需要比较 5 个字符
        if ((strncmp(KeyP, "true", 4) == 0) || (strncmp(KeyP, "TRUE", 4) == 0) || (strncmp(KeyP, "True", 4) == 0)) {
            ResBool = true;
        } else if ((strncmp(KeyP, "false", 5) == 0) || (strncmp(KeyP, "FALSE", 5) == 0) || (strncmp(KeyP, "False", 5) == 0)) {
            ResBool = false;
        }
    }
    return ResBool;
}
// 不支持原地转换, 避免破环 json 数据
static void Obj_getString(struct _JsonObject This, char Key[], strnew OutStr) {
    char *KeyP = getJsonValueStart(This.JsonString.Name._char, Key);
    if (KeyP == NULL) {
        return;
    }
    if (*KeyP != '"') { // 值的首字符不是引号, 不是字符串
        return;
    }
    // 从值的首字符之后开始扫描, 跳过转义字符, 避免把 \" 当作结束引号
    char *EndP = KeyP + 1;
    bool IsClosed = false;
    while ((EndP < (This.JsonString.Name._char + This.JsonString.MaxLen)) && (*EndP != '\0')) {
        if (*EndP == '\\') {
            EndP++;
            if (*EndP == '\0') {
                break;
            }
            EndP++;
            continue;
        }
        if (*EndP == '"') {
            IsClosed = true;
            break;
        }
        EndP++;
    }
    if (IsClosed == false) {
        return;
    }
    char Temp = *EndP;
    *EndP = '\0';
    memset(OutStr.Name._char, 0, OutStr.MaxLen);
    copyString(OutStr.Name._char, (KeyP + 1), OutStr.MaxLen, strlen(KeyP + 1));
    *EndP = Temp;
}
// 注意输出地址与原json字符串地址一致时, 会破坏原数据
static struct _JsonArray Obj_getArray(struct _JsonObject This, char Key[], strnew OutStr) {
    JsonArray tempJsonArr = newJsonArrayByString(OutStr);
    char *KeyP = getJsonValueStart(This.JsonString.Name._char, Key);
    if (KeyP != NULL) {
        char *EndP = NULL;
        if ((EndP = getDoubleChrOnString(KeyP, '[', ']')) != NULL) {
            char Temp = *(EndP + 1);
            *(EndP + 1) = '\0';
            if (OutStr.Name._char != This.JsonString.Name._char) {
                memset(tempJsonArr.JsonString.Name._char, 0, tempJsonArr.JsonString.MaxLen);
                copyString(tempJsonArr.JsonString.Name._char, KeyP, tempJsonArr.JsonString.MaxLen, strlen(KeyP));
                *(EndP + 1) = Temp;
            } else {
                tempJsonArr.JsonString.Name._char = KeyP;
                tempJsonArr.JsonString.MaxLen = strlen(KeyP);
            }
        }
    }
    return tempJsonArr;
}
// 注意输出地址与原json字符串地址一致时, 会破坏原数据
static struct _JsonObject Obj_getObject(struct _JsonObject This, char Key[], strnew OutStr) {
    JsonObject tempJsonObj = newJsonObjectByString(OutStr);
    char *KeyP = getJsonValueStart(This.JsonString.Name._char, Key);
    if (KeyP != NULL) {
        char *EndP = NULL;
        if ((EndP = getDoubleChrOnString(KeyP, '{', '}')) != NULL) {
            char Temp = *(EndP + 1);
            *(EndP + 1) = '\0';
            if (OutStr.Name._char != This.JsonString.Name._char) {
                memset(tempJsonObj.JsonString.Name._char, 0, tempJsonObj.JsonString.MaxLen);
                copyString(tempJsonObj.JsonString.Name._char, KeyP, tempJsonObj.JsonString.MaxLen, strlen(KeyP));
                *(EndP + 1) = Temp;
            } else {
                tempJsonObj.JsonString.Name._char = KeyP;
                tempJsonObj.JsonString.MaxLen = strlen(KeyP);
            }
        }
    }
    return tempJsonObj;
}

// // 判断是否符合 json 格式
// bool isJsonSrring(){
//     Stack s;
//     initStack(&s);

// }

JsonObject newJsonObjectByString(strnew DataInit) {
    JsonObject Temp;
    Temp.JsonString = DataInit;
    Temp.ItemNum = -1;
    Temp.sizeStr = Obj_sizeStr;
    Temp.isJsonNull = Obj_isJsonNull;
    Temp.getInt = Obj_getInt;
    Temp.getDouble = Obj_getDouble;
    Temp.getBool = Obj_getBool;
    Temp.getString = Obj_getString;
    Temp.getArray = Obj_getArray;
    Temp.getObject = Obj_getObject;
    return Temp;
}

//==========================================================================================//
//==========================================================================================//
#include "CheckCode.h"
#define CheckNum_T "CheckCode"
void addCsToJsonAndPushJsonStr(JsonObject InputJsonStrObj) {
    int AddrOver = strlen(InputJsonStrObj.JsonString.Name._char);
    if ((AddrOver + (int)strlen(",\"" CheckNum_T ":\":xxxxx") > InputJsonStrObj.JsonString.MaxLen) || (AddrOver < 1)) {
        return; // 空间不足以生成 cs 校验
    }
#if 0
    // 计算 cs 或其他检验算法
    uint16_t CheckNum = getModbusCrc(InputJsonStrObj.JsonString, AddrOver);
    addJsonItemData(InputJsonStrObj.JsonString, CheckNum_T ":%d", CheckNum);
    return;
#else
    uint8_t Sum = getSumCs(InputJsonStrObj.JsonString, AddrOver);
    addJsonItemData(InputJsonStrObj.JsonString, CheckNum_T ":%d", Sum);
    return;
#endif
}
bool checkOfCsJsonStrIsRight(strnew JsonInputStr, strnew JsonOutputStr) {
    JsonObject JsonObj = newJsonObjectByString(JsonInputStr);
    // 如果 NowCheckNum 不存在 直接退出
    if (JsonObj.isJsonNull(&JsonObj, CheckNum_T) < 0) {
        return false;
    }
    int NowCheckNum = JsonObj.getInt(&JsonObj, CheckNum_T);
    char *P_CheckStart = strstr(JsonObj.JsonString.Name._char, "\"" CheckNum_T "\":");
    char *P_CheckEnd = strstr(P_CheckStart, ",");
    if (P_CheckStart == NULL) {
        return false;
    }
    uint8_t CheckNumLocatFlag = 0;
    if (*(P_CheckStart - 1) == '{') { // 校验码在开头
        CheckNumLocatFlag = setDataBit(CheckNumLocatFlag, 0, 1);
    }
    if (P_CheckEnd == NULL) { // 校验码在结尾
        CheckNumLocatFlag = setDataBit(CheckNumLocatFlag, 1, 1);
    }

    if (CheckNumLocatFlag == 1) { // 校验码在开头
        (*P_CheckEnd) = '{';
        JsonInputStr.Name._char = P_CheckEnd;
        JsonInputStr.MaxLen -= (P_CheckEnd - P_CheckStart);
    } else if (CheckNumLocatFlag == 2) { // 校验码在结尾
        (*(P_CheckStart - 1)) = '}';
        (*P_CheckStart) = '\0';
    } else if (CheckNumLocatFlag == 3) { // 校验码在开头和结尾
        return false;
    } else {
        *(P_CheckStart) = '\0';
        catString(JsonObj.JsonString.Name._char, (P_CheckEnd + 1), JsonObj.JsonString.MaxLen, strlen((P_CheckEnd + 1)));
    }
    copyString(JsonOutputStr.Name._char, JsonInputStr.Name._char, JsonOutputStr.MaxLen, JsonInputStr.MaxLen);
    int DataLen = strlen(JsonOutputStr.Name._char);
    while (JsonOutputStr.Name._char[DataLen - 1] != '}' && JsonOutputStr.Name._char[DataLen - 1] != ']' && JsonOutputStr.Name._char[DataLen - 1] != '\0') {
        JsonOutputStr.Name._char[DataLen - 1] = '\0';
        DataLen--;
        if (DataLen == 0) {
            return false;
        }
    }
#if 0
    JsonInputStr.Name._char[DataLen++] = (((uint16_t)(NowCheckNum)&0xFF00) >> 8);
    JsonInputStr.Name._char[DataLen++] = (((uint16_t)(NowCheckNum)&0x00FF) >> 0);
    JsonOutputStr.Name._char[DataLen] = '\0';
    // 计算 cs 或其他检验算法
    return isModbusCrc(JsonInputStr, DataLen);
#else
    JsonOutputStr.Name._char[DataLen++] = NowCheckNum;
    JsonOutputStr.Name._char[DataLen] = '\0';
    return isSumCs(JsonOutputStr, DataLen);
#endif
}
