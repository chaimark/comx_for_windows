#include "SetTime.h"
// 对象 api
#define NowNode This.Head->prev
// 查找某个任务的地址
static Task_T *_getTaskByName(struct _timetask This, strnew Name) {
    Task_T *Temp = This.Head;
    while (strcmp((*Temp).Name.Name._char, Name.Name._char) != 0) {
        Temp = Temp->next;
    }
    return Temp;
}
// 添加某个任务节点
static int _addTaskNode(struct _timetask This, strnew Name) {
    Task_T *Temp = (Task_T *)malloc(sizeof(Task_T));
    if (Temp == NULL) {
        return -1;
    }
    This.NumberOfTimeTask++;
    // 添加数据
    (*Temp).Name = Name;
    Temp->isTaskStart = false; // 初始化标记
    Temp->TimeTask_Falge = false;
    Temp->CountNum = 0;    // 复位初始
    Temp->CountMaxNum = 0; // 复位初始
    Temp->TaskFunc = NULL;
    if (This.Head == NULL) {
        Temp->next = Temp;
        Temp->prev = Temp;
        This.Head = Temp;
        NowNode = This.Head;
    } else {
        Temp->next = This.Head;
        Temp->prev = NowNode;
        NowNode->next = Temp;
        This.Head->prev = Temp;
    }
    return 0;
}
// 初始化某个任务
static void _initTaskByName(struct _timetask This, strnew Name, uint64_t CountMaxNum, void (*TaskFunc)(void *), void *arg) {
    Task_T *Temp = _getTaskByName(&This, Name);
    Temp->TimeTask_Falge = (CountMaxNum == 0 ? true : false); // 初始化标记
    Temp->isTaskStart = true;                                 // 开启
    Temp->CountMaxNum = CountMaxNum;                          // 定时任务点
    Temp->CountNum = 0;                                       // 复位初始
    Temp->arg = arg;
    Temp->TaskFunc = TaskFunc;
    if (Temp->TimeTask_Falge == true) {
        if (Temp->TaskFunc != NULL) {
            Temp->TaskFunc(arg); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
        }
    }
}
// 暂停某个任务
static void _stopTaskByName(struct _timetask This, strnew Name) {
    Task_T *Temp = _getTaskByName(&This, Name);
    Temp->isTaskStart = false; // 初始化标记
    Temp->TimeTask_Falge = false;
    Temp->CountNum = 0; // 复位初始
    Temp->TaskFunc = NULL;
}
// 关闭所有任务,清理所有的链表节点
static void _closeTaskAll(struct _timetask This) {
    if (This.Head != NULL) {
        Task_T *cur = This.Head;
        Task_T *next = NULL;
        do {
            next = cur->next; // 先记住下一个节点
            free(cur);        // 释放当前节点的堆内存
            cur = next;
        } while (cur != This.Head); // 当 cur 绕了一圈回到 Head 时，安全跳出循环
        (This).Head = NULL;
    }
    (This).NumberOfTimeTask = 0;
}
// 初始化
timetask initSetTime() {
    timetask TaskInit = {0};
    TaskInit.Head = NULL;
    TaskInit.getTaskByName = _getTaskByName;
    TaskInit.addTaskNode = _addTaskNode;
    TaskInit.initTaskByName = _initTaskByName;
    TaskInit.stopTaskByName = _stopTaskByName;
    TaskInit.closeTaskAll = _closeTaskAll;
    return TaskInit;
}
// 计数函数
void CountSetTimeTask(timetask This) {
    Task_T *Temp = This.Head;
    for (int TaskAddr = 0; TaskAddr < This.NumberOfTimeTask; TaskAddr++) {
        if ((*Temp).isTaskStart == false) {
            Temp = Temp->next;
            continue;
        }
        if ((*Temp).CountNum < (*Temp).CountMaxNum) {
            Temp->CountNum++;
        }
        if ((*Temp).CountNum >= (*Temp).CountMaxNum) {
            (*Temp).TimeTask_Falge = true;
        } else {
            (*Temp).TimeTask_Falge = false;
        }
        if ((*Temp).TimeTask_Falge == true) {
            if ((*Temp).TaskFunc != NULL) {
                (*Temp).TaskFunc(Temp->arg); // 注意:该函数, 执行时不要太长, 也不要启动同一个定时器的其他任务
            }
        }
        Temp = Temp->next;
    }
}
