#include "StatusModel.h"

// 查找某个任务的地址
static StatusTaskFunc *_getTaskById(struct _StatusDev This, uint16_t InputId) {
    StatusTaskFunc *Temp = This.Head;
    int i = 0;
    for (i = 0; i < This.NumberOfStatus; i++) {
        if ((*Temp).Id == InputId) {
            break;
        }
        Temp = Temp->next;
    }
    if (i < This.NumberOfStatus) {
        return Temp;
    }
    return NULL;
}

// 添加某个任务函数节点
int _addTaskFuncNode(struct _StatusDev This, uint16_t InputId, void (*TaskFunc)(void *), void *arg) {
    // 检查是否存在该任务
    StatusTaskFunc *Temp = _getTaskById(&This, InputId);
    // 如果存在, 则返回 -1
    if (Temp != NULL) {
        return -1;
    }
    StatusTaskFunc *Temp = (StatusTaskFunc *)malloc(sizeof(StatusTaskFunc));
    if (Temp == NULL) {
        return -1;
    }
    This.NumberOfStatus++;
    // 添加数据
    Temp->Id = InputId;
    Temp->TaskFunc = TaskFunc;
    Temp->arg = arg;
    if (This.Head == NULL) {
        Temp->next = Temp;
        Temp->prev = Temp;
        This.Head = Temp;
        This.Head->prev = This.Head;
    } else {
        Temp->next = This.Head;
        Temp->prev = This.Head->prev;
        This.Head->prev->next = Temp;
        This.Head->prev = Temp;
    }
    return 0;
}

// 删除某个任务函数节点
void _delTaskFuncById(struct _StatusDev This, uint16_t InputId) {
    StatusTaskFunc *Temp = _getTaskById(&This, InputId);
    if (Temp == NULL) {
        return;
    }
    if (This.NumberOfStatus < 1) {
        return;
    }
    if (This.NumberOfStatus == 1) {
        This.Head = NULL;
        This.NumberOfStatus = 0;
        free(Temp);
        return;
    }
    Temp->prev->next = Temp->next;
    Temp->next->prev = Temp->prev;
    This.NumberOfStatus--;
    free(Temp);
    return;
}

// 关闭所有任务
void _closeTaskFuncAll(struct _StatusDev This) {
    if (This.Head != NULL) {
        StatusTaskFunc *cur = This.Head;
        StatusTaskFunc *next = NULL;
        do {
            next = cur->next; // 先记住下一个节点
            free(cur);        // 释放当前节点的堆内存
            cur = next;
        } while (cur != This.Head); // 当 cur 绕了一圈回到 Head 时, 安全跳出循环
        (This).Head = NULL;
    }
    (This).NumberOfStatus = 0;
}

// 执行某个任务
void _ExecuteStep(struct _StatusDev This) {
    if (This.IsRun == false) {
        return;
    }
    StatusTaskFunc *Temp = _getTaskById(&This, This.NextId);
    if ((Temp != NULL) && (Temp->TaskFunc != NULL)) {
        Temp->TaskFunc(Temp->arg);
    }
}

// 初始化
StatusDev initStatusDev(void) {
    StatusDev TempStatusDev = {0};
    TempStatusDev.addTaskFuncNode = _addTaskFuncNode;
    TempStatusDev.delTaskFuncById = _delTaskFuncById;
    TempStatusDev.closeTaskFuncAll = _closeTaskFuncAll;
    TempStatusDev.ExecuteStep = _ExecuteStep;
    TempStatusDev.NextId = 1;
    TempStatusDev.IsRun = true;
    return TempStatusDev;
}