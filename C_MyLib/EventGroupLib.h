#ifndef __EVENTGROUPLIB__
#define __EVENTGROUPLIB__

#include "./StrLib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <string.h>

typedef struct _itemevent {
    strnew Name;
    int efd;
    struct _itemevent *next;
    struct _itemevent *prev;
} itemevent_t;
typedef struct _EventGroup {
    // epoll 监控文件句柄
    int epfd;
    struct epoll_event ev;
    // 事件节点
    itemevent_t *Head;
    // 事件总数
    int EventsNumber;
    // 添加事件
    int (*addEvent)(struct _EventGroup This, strnew Name);
    // 等待某个事件（阻塞）
    int (*waitEventForName)(struct _EventGroup This, strnew Name);
    // 检查是否有事件产生
    int (*checkEvents)(struct _EventGroup This, struct epoll_event *Events, uint8_t MaxEventNum);
} eventGroup;

// 定义一个事件组
extern eventGroup newEventGroup(void);
// 删除所有事件
extern void cleanEventGroup(eventGroup This);

// 安全宏 - 防止忘记释放
#define eventGroup_malloc(Name) \
    __attribute__((cleanup(cleanEventGroup))) eventGroup Name = newEventGroup();

#endif
