#ifndef __TEST__
#define __TEST__

#include "./StrLib.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct _PRIVATE PRIVATE;
typedef struct _testClass {
    // 私有
    PRIVATE *pdata;
    // 公共
    int a;
    int b;
    int (*add)(struct _testClass This);
    int (*getK)(struct _testClass This);
    void (*setK)(struct _testClass This, int setVar);
} testClass;

// 基础API
extern testClass newTestClass(void);
extern void cleanTestClass(testClass This);

// 安全宏 - 防止忘记释放
#define testClass_malloc(name) \
    __attribute__((cleanup(cleanTestClass))) testClass name = newTestClass();
#define SCOPE_testClass(name, code)      \
    do {                                 \
        testClass name = newTestClass(); \
        code;                            \
        cleanTestClass(&name);           \
    } while (0)

#endif
