#ifndef __CJSON__
#define __CJSON__

/*
 * JsonArray_T 和 JsonObject_T 中的 pdata 是私有变量指针，
 * 在 newJsonArrayByString 中 malloc 申请一个 int。
 * 使用 JsonArray 定义时会自动 free 申请的空间。
 *
 * 安全使用规则: JsonObject
 * 1. newJsonXXXXXXByString 过的 JsonArray/JsonObject 不能被覆盖
 * 2. 函数局部 JsonArray/JsonObject 赋值给全局变量 JsonArray_T/JsonObject_T 时,
 *    必须使用 函数 reSetJsonArray()/reSetJsonObject() 转换成 JsonObject_T
 *    reSetJsonArray()/reSetJsonObject() 转换后 pdata == NULL 无需关心 pdata,
 *    私有变量 pdata 不允许访问，JsonArray_T 也不会被 free,
 *    This.sizeItemNum() 与 Thisi.sizeStr() 访问 pdata == NULL
 */


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
static inline JsonArray_T reSetJsonArray(JsonArray_T InputClass) {
    JsonArray_T empty = InputClass;
    empty.pdata = NULL;
    return empty;
}
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
static inline JsonObject_T reSetJsonObject(JsonObject_T InputClass) {
    JsonObject_T empty = InputClass;
    empty.pdata = NULL;
    return empty;
}
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
