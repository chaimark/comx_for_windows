#include "test.h"

typedef struct _PRIVATE {
    int vark;
} PRIVATE;

int _add(struct _testClass This) {
    return This.a + This.b;
}

// 通过函数暴露 私有变量
int _getK(struct _testClass This) {
    return This.pdata->vark;
}
void _setK(struct _testClass This, int setVar) {
    This.pdata->vark = setVar;
}

testClass new_testClass(void) {
    testClass Temp = {0};
    Temp.a = 0;
    Temp.b = 0;
    Temp.pdata = (struct _PRIVATE*)malloc(sizeof(struct _PRIVATE));
    Temp.pdata->vark = 0;
    Temp.add = _add;
    Temp.getK = _getK;
    Temp.setK = _setK;
    return Temp;
}

void clean_testClass(testClass This) {
    // 释放私有数据的内存
    if (This.pdata) {
        free(This.pdata);
        This.pdata = NULL;
    }
}
