#ifndef __TEST__
#define __TEST__

#include "./StrLib.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct _PRIVATE PRIVATE;
typedef struct _testClass {
    // 私有
    PRIVATE* pdata;
    // 公共
    int a;
    int b;
    int (*add)(struct _testClass This);
    int (*getK)(struct _testClass This);
    void (*setK)(struct _testClass This, int setVar);
} testClass;

// 基础API
testClass newTestClass(void);
void cleanTestClass(testClass This);

// 安全宏 - 防止忘记释放
#define AUTO_TestClass __attribute__((cleanup(clean_testClass))) testClass
#define SCOPE_testClass(name, code)                                                                                    \
    do {                                                                                                               \
        testClass name = new_testClass();                                                                              \
        code;                                                                                                          \
        clean_testClass(&name);                                                                                        \
    } while (0)

#endif
