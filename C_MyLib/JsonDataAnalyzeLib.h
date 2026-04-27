#ifndef __JSONDATAANALYZELIB_H__
#define __JSONDATAANALYZELIB_H__

#include "StrLib.h"
#include <stdint.h>
// Json数组类
typedef struct _JsonArray {
    strnew JsonString;
    int    ItemNum;
    int (*sizeItemNum)(struct _JsonArray This);                           // 获取Json 数组个数
    signed char (*isJsonNull)(struct _JsonArray This);                    // 判断Json 是否为空
    void (*get)(struct _JsonArray This, strnew OutStr, int ItemNum);      // 获取Json 某元素
    void (*getArray)(struct _JsonArray This, strnew OutStr, int ItemNum); // 获取Json 某元素
} JsonArray;
// 建立对象示例
extern JsonArray newJsonArrayByString(strnew DataInit); // 建立对象数组的函数

//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//

// json类
typedef struct _JsonObject {
    strnew JsonString;
    int (*sizeStr)(struct _JsonObject This);                                           // 获取Json 键值对的个数
    signed char (*isJsonNull)(struct _JsonObject This, char Key[]);                    // 判断Json 是否为空
    int (*getInt)(struct _JsonObject This, char Key[]);                                // 获取Json int
    double (*getDouble)(struct _JsonObject This, char Key[]);                          // 获取Json double
    bool (*getBool)(struct _JsonObject This, char Key[]);                              // 获取Json bool
    void (*getString)(struct _JsonObject This, char Key[], strnew OutStr);             // 获取Json string
    struct _JsonArray (*getArray)(struct _JsonObject This, char Key[], strnew OutStr); // 获取Json array
    struct _JsonObject (*getObject)(struct _JsonObject This, char Key[], strnew OutStr); // 获取Json object
} JsonObject;
// 建立对象示例
extern JsonObject newJsonObjectByString(strnew DataInit); // 建立对象的函数


#endif
