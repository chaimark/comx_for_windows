#include "JsonSetLib.h"
#include "JsonDataAnalyzeLib.h"
#include <stdarg.h>
#include <stdio.h>


// #define IsOpenFloatHelp_Ability

#ifdef IsOpenFloatHelp_Ability
char getNowType(const char *NowAddr, char *UserFromNow) {
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

bool getFromTypeCheckDoubleOrFloat(strnew FromStr) {
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
