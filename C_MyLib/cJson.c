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
    int ReFlag = 1; // 先预留，并寻找最后一个字符 '}'
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
    char *Addr_OverName = strchr(FromStr, ':');
    const char *Addr_Start = strchr(FromStr, ',');
    if ((Addr_Start != NULL) && (Addr_Start < Addr_OverName)) {
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
        while (!((JsonStringSpace.Name._char[NowStrLen] == '\0') &&
                 (JsonStringSpace.Name._char[NowStrLen + 1] == '\0') &&
                 (JsonStringSpace.Name._char[NowStrLen + 2] == '\0'))) {
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
// 获取栈顶元素
static char peek(Stack *s) {
    if (!isEmpty(s)) {
        return s->data[s->top];
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
    for (int i = 0; (MotherString[i] != '\0'); i++) {
        if (MotherString[i] == HeadChr) {
            push(&s, HeadChr);
        } else if (MotherString[i] == EndChr) {
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

// 拼接 关键字 字符串
static void _getKeyName(strnew SonStr, char Key[]) {
    catString(SonStr.Name._char, "\"", SonStr.MaxLen, 1);
    catString(SonStr.Name._char, Key, SonStr.MaxLen, strlen(Key));
    catString(SonStr.Name._char, "\":", SonStr.MaxLen, 2);
}
#define getKeyName(name, len, key) \
    char name[len] = {0};          \
    _getKeyName(NEW_NAME(name), key)

//==========================================================================================//
JsonArray_T newJsonArrayByString(strnew DataInit);
JsonObject_T newJsonObjectByString(strnew DataInit);
//==========================================================================================//
static int Arr_sizeItemNum(struct _JsonArray_T This) {
    if (This.ItemNum != -1) {
        return This.ItemNum;
    }
    Stack s;       // 定义栈
    initStack(&s); // 初始化栈
    int ItemNum = 0;
    char *HeadItem = This.JsonString.Name._char + 1; // 获取第一个元素位置
    char *EndItem = HeadItem;
    bool isStringArray = false;
    if (myStrstrCont(This.JsonString.Name._char, "\"", This.JsonString.MaxLen, 2) != NULL) {
        isStringArray = true;
    }
    while (*EndItem != '\0') {
        if (isEmpty(&s)) {                            // 判断当前栈是否为空，
            if (*EndItem == '{' || *EndItem == '[') { // 如果是 { 或者 [ 就入栈，
                push(&s, *EndItem);
            } else if (isStringArray == false) {
                if ((*EndItem == ',') || (*(EndItem + 1) == '\0')) { // 如果为空，则判断 EndItem 是否是‘,’
                    ItemNum++;                                       // 如果是‘,’，则说明是一个元素结束 ItemNum++;
                    HeadItem = EndItem + 1;
                    EndItem = HeadItem;
                    continue;
                }
            } else {
                if (((*(EndItem - 1) == '\"') && (*EndItem == ',')) ||
                    (*(EndItem + 1) == '\0')) { // 如果为空，则判断 EndItem 是否是‘,’
                    ItemNum++;                  // 如果是‘,’，则说明是一个元素结束 ItemNum++;
                    HeadItem = EndItem + 1;
                    EndItem = HeadItem;
                    continue;
                }
            }
        } else {
            if ((*EndItem == '}' || *EndItem == ']') && ((peek(&s) == '{') || (peek(&s) == '['))) { // 如果当前栈不为空，则在遇到 } 或者 ] 出栈，
                pop(&s);
            }
        }
        EndItem++;
    }
    This.ItemNum = (This.isJsonNull(&This) ? 0 : ItemNum);
    return ItemNum;
}
static signed char Arr_isJsonNull(struct _JsonArray_T This) {
    char *StartP = NULL;
    StartP = strchr(This.JsonString.Name._char, '[');
    if (StartP == NULL) {
        return false;
    }
    int i = 0;
    while (!(StartP[i] != ' ' && StartP[i] != '\t') && (StartP[i] != '\0') && (StartP[i] != ']')) {
        i++;
    }
    return ((StartP[i] != ']' && StartP[i] != '\0') ? false : true);
}

static void Arr_get(struct _JsonArray_T This, strnew OutStr, int ItemNum) {
    Stack s;       // 定义栈
    initStack(&s); // 初始化栈
    ItemNum++;
    char *HeadItem = This.JsonString.Name._char + 1; // 获取第一个元素位置
    char *EndItem = HeadItem;
    bool isStringArray = false;
    if (myStrstrCont(This.JsonString.Name._char, "\"", This.JsonString.MaxLen, 2) != NULL) {
        isStringArray = true;
    }
    while (ItemNum > 0 && *EndItem != '\0') {
        if (isEmpty(&s)) {                            // 判断当前栈是否为空，
            if (*EndItem == '{' || *EndItem == '[') { // 如果是 { 或者 [ 就入栈，
                push(&s, *EndItem);
            } else if (isStringArray == false) {
                if ((*EndItem == ',') || (*(EndItem + 1) == '\0')) { // 如果为空，则判断 EndItem 是否是‘,’
                    ItemNum--;                                       // 如果是‘,’，则说明是一个元素结束 ItemNum--;
                    if (ItemNum != 0) {
                        HeadItem = EndItem + 1;
                        EndItem = HeadItem;
                        continue;
                    }
                }
            } else {
                if (((*(EndItem - 1) == '\"') && (*EndItem == ',')) ||
                    (*(EndItem + 1) == '\0')) { // 如果为空，则判断 EndItem 是否是‘,’
                    ItemNum--;                  // 如果是‘,’，则说明是一个元素结束 ItemNum--;
                    if (ItemNum != 0) {
                        HeadItem = EndItem + 1;
                        EndItem = HeadItem;
                        continue;
                    }
                }
            }
        } else {
            if (*EndItem == '}' || *EndItem == ']') { // 如果当前栈不为空，则在遇到 } 或者 ] 出栈，
                pop(&s);
            }
        }
        EndItem++;
    }
    char Temp = *(EndItem - 1);
    *(EndItem - 1) = '\0';
    memset(OutStr.Name._char, 0, OutStr.MaxLen);
    copyString(OutStr.Name._char, HeadItem, OutStr.MaxLen, strlen(HeadItem));
    *(EndItem - 1) = Temp;
    int NowLineMaxLen = strlen(OutStr.Name._char);
    // 找第一个非空字符
    for (int i = 0; i < NowLineMaxLen; i++) {
        if ((OutStr.Name._char[i] != '\0') &&
            (OutStr.Name._char[i] != '\t') && (OutStr.Name._char[i] != '[') && (OutStr.Name._char[i] != ' ') &&
            (OutStr.Name._char[i] != '\r') && (OutStr.Name._char[i] != '\n') && (OutStr.Name._char[i] != '\"')) {
            HeadItem = &OutStr.Name._char[i];
            break;
        }
    }
    // 找最后一个非空指挥
    for (int i = (NowLineMaxLen - 1); i > 0; i--) {
        if ((OutStr.Name._char[i] != '\0') &&
            (OutStr.Name._char[i] != '\t') && (OutStr.Name._char[i] != '[') && (OutStr.Name._char[i] != ' ') &&
            (OutStr.Name._char[i] != '\r') && (OutStr.Name._char[i] != '\n') && (OutStr.Name._char[i] != '\"')) {
            EndItem = &OutStr.Name._char[i];
            break;
        }
    }
    *(EndItem + 1) = '\0';
    size_t GetStrLen = 0;
    for (GetStrLen = 0; GetStrLen < strlen(HeadItem); GetStrLen++) {
        OutStr.Name._char[GetStrLen] = *(HeadItem + GetStrLen);
    }
    if (GetStrLen < (size_t)OutStr.MaxLen) {
        OutStr.Name._char[GetStrLen] = '\0';
    } else {
        OutStr.Name._char[OutStr.MaxLen - 1] = '\0';
    }
}
static void Arr_getArray(struct _JsonArray_T This, strnew OutStr, int ItemNum) {
    if (OutStr.Name._char == This.JsonString.Name._char) {
        return;
    }
    OutStr.Name._char[0] = '[';
    OutStr.Name._char += 1;
    OutStr.MaxLen -= 1;
    This.get(&This, OutStr, ItemNum);
    catString(OutStr.Name._char, "]", OutStr.MaxLen, 1);
    OutStr.Name._char -= 1;
    OutStr.MaxLen += 1;
    OutStr.Name._char[strlen(OutStr.Name._char)] = '\0';
}
JsonArray_T newJsonArrayByString(strnew DataInit) {
    JsonArray_T Temp;
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

static int Obj_sizeStr(struct _JsonObject_T This) {
    // 未完成 用于计算 obj 的键值对
    (void)_THIS_MY_;
    return 0;
}
static signed char Obj_isJsonNull(struct _JsonObject_T This, char Key[]) {
    signed char ResOver = -1;
    getKeyName(SonStr, 50, Key);
    char *KeyP = NULL;
    if ((KeyP = strstr(This.JsonString.Name._char, SonStr)) != NULL) {
        ResOver = false;
        KeyP += strlen(SonStr);
        while ((*KeyP) == ' ') {
            KeyP++;
        }
        char Temp = *(KeyP + 4);
        *(KeyP + 4) = '\0';
        if ((strcmp(KeyP, "null") == 0) || (strcmp(KeyP, "NULL") == 0) || (strcmp(KeyP, "Null") == 0)) {
            ResOver = true;
        }
        *(KeyP + 4) = Temp;
    }
    return ResOver;
}
static int Obj_getInt(struct _JsonObject_T This, char Key[]) {
    getKeyName(SonStr, 50, Key);
    char *KeyP = NULL;
    if ((KeyP = strstr(This.JsonString.Name._char, SonStr)) != NULL) {
        KeyP += strlen(SonStr);
        while ((*KeyP) == ' ') {
            KeyP++;
        }
        return doneAsciiStrToAnyBaseNumberData(KeyP, 16);
        // return atol(KeyP);
    }
    return 0;
}
static double Obj_getDouble(struct _JsonObject_T This, char Key[]) {
    getKeyName(SonStr, 50, Key);
    char *KeyP = NULL;
    if ((KeyP = strstr(This.JsonString.Name._char, SonStr)) != NULL) {
        KeyP += strlen(SonStr);
        while ((*KeyP) == ' ') {
            KeyP++;
        }
        return atof(KeyP);
    }
    return 0.0;
}
static bool Obj_getBool(struct _JsonObject_T This, char Key[]) {
    bool ResBool = false;
    getKeyName(SonStr, 50, Key);
    char *KeyP = NULL;
    if ((KeyP = strstr(This.JsonString.Name._char, SonStr)) != NULL) {
        KeyP += strlen(SonStr);
        while ((*KeyP) == ' ') {
            KeyP++;
        }
        char Temp = *(KeyP + 4);
        *(KeyP + 4) = '\0';
        if ((strcmp(KeyP, "true") == 0) || (strcmp(KeyP, "TRUE") == 0) || (strcmp(KeyP, "True") == 0)) {
            ResBool = true;
        } else if ((strcmp(KeyP, "false") == 0) || (strcmp(KeyP, "FALSE") == 0) || (strcmp(KeyP, "False") == 0)) {
            ResBool = false;
        }
        *(KeyP + 4) = Temp;
    }
    return ResBool;
}
// 不支持原地转换，避免破环 json 数据
static void Obj_getString(struct _JsonObject_T This, char Key[], strnew OutStr) {
    getKeyName(SonStr, 50, Key);
    char *KeyP = NULL;
    if ((KeyP = strstr(This.JsonString.Name._char, SonStr)) != NULL) {
        KeyP += strlen(SonStr);
        while ((*KeyP) == ' ') {
            KeyP++;
        }
        char *EndP = ++KeyP;
        while (EndP - This.JsonString.Name._char < This.JsonString.MaxLen) {
            EndP = strchr(EndP, '"');
            if (*(EndP - 1) != '\\') {
                break;
            } else {
                EndP++;
            }
        }
        char Temp = *EndP;
        *EndP = '\0';
        memset(OutStr.Name._char, 0, OutStr.MaxLen);
        copyString(OutStr.Name._char, KeyP, OutStr.MaxLen, strlen(KeyP));
        *EndP = Temp;
    }
    return;
}
// 注意输出地址与原json字符串地址一致时，会破坏原数据
static struct _JsonArray_T Obj_getArray(struct _JsonObject_T This, char Key[], strnew OutStr) {
    JsonArray_T tempJsonArr = newJsonArrayByString(OutStr);
    getKeyName(SonStr, 50, Key);
    char *KeyP = NULL;
    if ((KeyP = strstr(This.JsonString.Name._char, SonStr)) != NULL) {
        KeyP += strlen(SonStr);
        while ((*KeyP) == ' ') {
            KeyP++;
        }
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
// 注意输出地址与原json字符串地址一致时，会破坏原数据
static struct _JsonObject_T Obj_getObject(struct _JsonObject_T This, char Key[], strnew OutStr) {
    JsonObject_T tempJsonObj = newJsonObjectByString(OutStr);
    getKeyName(SonStr, 50, Key);
    char *KeyP = NULL;
    if ((KeyP = strstr(This.JsonString.Name._char, SonStr)) != NULL) {
        KeyP += strlen(SonStr);
        while ((*KeyP) == ' ') {
            KeyP++;
        }
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

JsonObject_T newJsonObjectByString(strnew DataInit) {
    JsonObject_T Temp;
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
void addCsToJsonAndPushJsonStr(JsonObject_T InputJsonStrObj) {
    int CheckNum = 0;
    int AddrOver = strlen(InputJsonStrObj.JsonString.Name._char);
    if ((AddrOver + (int)strlen(",\"NowCheckNum\":xxxx") > InputJsonStrObj.JsonString.MaxLen) || (AddrOver < 1)) {
        return; // 空间不足以生成 cs 校验
    }
    // 计算 cs 或其他检验算法
    for (int i = 0; i < AddrOver; i++) {
        CheckNum += InputJsonStrObj.JsonString.Name._char[i];
    }
    addJsonItemData(InputJsonStrObj.JsonString, "NowCheckNum:%d", (CheckNum % 256));
    return;
}
bool checkOfCsJsonStrIsRight(strnew JsonInputStr, strnew JsonOutputStr) {
    JsonObject_T JsonObj = newJsonObjectByString(JsonInputStr);
    // 如果 NowCheckNum 不存在 直接退出
    if (JsonObj.isJsonNull(&JsonObj, "NowCheckNum") < 0) {
        return false;
    }
    int NowCheckNum = JsonObj.getInt(&JsonObj, "NowCheckNum");
    char *PEnd = strstr(JsonObj.JsonString.Name._char, ",\"NowCheckNum\"");
    if ((PEnd == NULL) || (PEnd >= JsonObj.JsonString.Name._char + JsonObj.JsonString.MaxLen - 1)) {
        return false;
    }
    (*(PEnd++)) = '}';
    (*PEnd) = '\0';
    // 计算 cs 或其他检验算法
    int CheckNum = 0;
    for (size_t i = 0; i < strlen(JsonInputStr.Name._char); i++) {
        CheckNum += JsonInputStr.Name._char[i];
    }
    if (JsonInputStr.Name._char != JsonOutputStr.Name._char) {
        copyString(JsonOutputStr.Name._char, JsonInputStr.Name._char, JsonOutputStr.MaxLen, JsonInputStr.MaxLen);
    }
    return ((CheckNum == NowCheckNum) ? true : false);
}
