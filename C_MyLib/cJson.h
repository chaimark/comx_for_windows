#ifndef __CJSON__
#define __CJSON__

#include "StrLib.h"
#include <stdint.h>
/***************** Json 处理 *****************/
// Json数组类
typedef struct _PRIVATE PRIVATE;
typedef struct _JsonArray_T {
    strnew JsonString;
    PRIVATE *pdata;
    int (*sizeItemNum)(struct _JsonArray_T This);                           // 获取Json 数组个数
    signed char (*isJsonNull)(struct _JsonArray_T This);                    // 判断Json 是否为空
    void (*get)(struct _JsonArray_T This, strnew OutStr, int ItemNum);      // 获取Json 某元素
    void (*getArray)(struct _JsonArray_T This, strnew OutStr, int ItemNum); // 获取Json 某元素
} JsonArray_T;
// 建立对象示例
extern JsonArray_T newJsonArrayByString(strnew DataInit); // 建立对象数组的函数
extern void cleanJsonArrayClass(JsonArray_T *_ClearPrt_);
#define JsonArray __attribute__((cleanup(cleanJsonArrayClass))) JsonArray_T

//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//

// json类
typedef struct _JsonObject_T {
    strnew JsonString;
    PRIVATE *pdata;
    int (*sizeStr)(struct _JsonObject_T This);                                               // 获取Json 键值对的个数
    signed char (*isJsonNull)(struct _JsonObject_T This, char Key[]);                        // 判断Json 是否为空
    int (*getInt)(struct _JsonObject_T This, char Key[]);                                    // 获取Json int
    double (*getDouble)(struct _JsonObject_T This, char Key[]);                              // 获取Json double
    bool (*getBool)(struct _JsonObject_T This, char Key[]);                                  // 获取Json bool
    void (*getString)(struct _JsonObject_T This, char Key[], strnew OutStr);                 // 获取Json string
    struct _JsonArray_T (*getArray)(struct _JsonObject_T This, char Key[], strnew OutStr);   // 获取Json array
    struct _JsonObject_T (*getObject)(struct _JsonObject_T This, char Key[], strnew OutStr); // 获取Json object
} JsonObject_T;
// 建立对象示例
extern JsonObject_T newJsonObjectByString(strnew DataInit); // 建立对象的函数
extern void cleanJsonObjectClass(JsonObject_T *_ClearPrt_);
#define JsonObject __attribute__((cleanup(cleanJsonObjectClass))) JsonObject_T

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
extern void addCsToJsonAndPushJsonStr(JsonObject_T InputJsonStrObj);
extern bool checkOfCsJsonStrIsRight(strnew JsonInputStr, strnew JsonOutputStr);

#endif
