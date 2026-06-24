#ifndef __CJSON__
#define __CJSON__

#include "StrLib.h"
#include <stdint.h>
/***************** Json 处理 *****************/
// Json数组类
typedef struct _PRIVATE PRIVATE;
typedef struct _JsonArray {
    strnew JsonString;
    int ItemNum;
    int (*sizeItemNum)(struct _JsonArray This);                           // 获取Json 数组个数
    signed char (*isJsonNull)(struct _JsonArray This);                    // 判断Json 是否为空
    void (*get)(struct _JsonArray This, strnew OutStr, int ItemNum);      // 获取Json 某元素
    void (*getArray)(struct _JsonArray This, strnew OutStr, int ItemNum); // 获取Json 某元素
} JsonArray;
// 建立对象示例
extern JsonArray newJsonArrayByString(strnew DataInit); // 建立对象数组的函数
extern void cleanJsonArrayClass(JsonArray *_ClearPrt_);

//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//

// json类
typedef struct _JsonObject {
    strnew JsonString;
    int ItemNum;
    int (*sizeStr)(struct _JsonObject This);                                             // 获取Json 键值对的个数
    signed char (*isJsonNull)(struct _JsonObject This, char Key[]);                      // 判断Json 是否为空
    int (*getInt)(struct _JsonObject This, char Key[]);                                  // 获取Json int
    double (*getDouble)(struct _JsonObject This, char Key[]);                            // 获取Json double
    bool (*getBool)(struct _JsonObject This, char Key[]);                                // 获取Json bool
    void (*getString)(struct _JsonObject This, char Key[], strnew OutStr);               // 获取Json string
    struct _JsonArray (*getArray)(struct _JsonObject This, char Key[], strnew OutStr);   // 获取Json array
    struct _JsonObject (*getObject)(struct _JsonObject This, char Key[], strnew OutStr); // 获取Json object
} JsonObject;
// 建立对象示例
extern JsonObject newJsonObjectByString(strnew DataInit); // 建立对象的函数
extern void cleanJsonObjectClass(JsonObject *_ClearPrt_);

/***************** Json 构建 *****************/
extern void addJsonItemData(strnew JsonStringSpace, const char *FromStr, ...);
#define newRootJsonObject(JsonStringSpace) JsonStringSpace.Name._char[0] = '{'
#define newRootJsonArray(JsonStringSpace) JsonStringSpace.Name._char[0] = '['

#define newClassJsonItem(JsonStringSpace, name, code, ClassType_1, ClassType_2)           \
    do {                                                                                  \
        if (strlen(name) == 0) {                                                          \
            addJsonItemData(JsonStringSpace, ClassType_1);                                \
        } else {                                                                          \
            addJsonItemData(JsonStringSpace, name ":" ClassType_1);                       \
        }                                                                                 \
        code;                                                                             \
        catString(JsonStringSpace.Name._char, ClassType_2, JsonStringSpace.MaxLen, 1);    \
        char *lastAddr = &JsonStringSpace.Name._char[strlen(JsonStringSpace.Name._char)]; \
        swapChr((lastAddr - 1), (lastAddr - 2));                                          \
    } while (0)

#define newSubObjectJsonItem(JsonStringSpace, name, code) newClassJsonItem(JsonStringSpace, name, code, "{", "}")
#define newSubArrayJsonItem(JsonStringSpace, name, code) newClassJsonItem(JsonStringSpace, name, code, "[", "]")

/***************** Json 校验 *****************/
extern void addCsToJsonAndPushJsonStr(JsonObject InputJsonStrObj);
extern bool checkOfCsJsonStrIsRight(strnew JsonInputStr, strnew JsonOutputStr);

#endif
