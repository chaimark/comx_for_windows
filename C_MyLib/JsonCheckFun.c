#include "JsonCheckFun.h"
#include "stdio.h"

void addCsToJsonAndPushJsonStr(JsonObject InputJsonStrObj) {
    int CheckNum = 0;
    int AddrOver = strlen(InputJsonStrObj.JsonString.Name._char);
    if ((AddrOver + (int)strlen(",\"NowCheckNum\":xxxx") > InputJsonStrObj.JsonString.MaxLen) || (AddrOver < 1)) {
        return; // 空间不足以生成 cs 校验
    }
    // 计算 cs 或其他检验算法
    for (int i = 0; i < AddrOver; i++) {
        CheckNum += InputJsonStrObj.JsonString.Name._char[i];
    }
    InputJsonStrObj.JsonString.Name._char[AddrOver - 1] = '\0';
    addJsonItemData(InputJsonStrObj.JsonString, ",");
    addJsonItemData(InputJsonStrObj.JsonString, "NowCheckNum:%d", (CheckNum % 256));
    addJsonItemData(InputJsonStrObj.JsonString, "}");
    return;
}
bool checkOfCsJsonStrIsRight(strnew JsonInputStr, strnew JsonOutputStr) {
    JsonObject JsonObj = newJsonObjectByString(JsonInputStr);
    // 如果 NowCheckNum 不存在 直接退出
    if (JsonObj.isJsonNull(&JsonObj, "NowCheckNum") < 0) {
        return false;
    }
    int   NowCheckNum = JsonObj.getInt(&JsonObj, "NowCheckNum");
    char* PEnd = strstr(JsonObj.JsonString.Name._char, ",\"NowCheckNum\"");
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
