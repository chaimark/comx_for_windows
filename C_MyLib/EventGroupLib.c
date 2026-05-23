#include "EventGroupLib.h"
#define NowNode This.Head->prev
// 添加事件
int _addEvent(struct _EventGroup This, strnew Name) {
    itemevent_t *Temp = (itemevent_t *)malloc(sizeof(itemevent_t));
    if (Temp == NULL) {
        return -1;
    }
    // 添加数据
    (*NowNode).Name = Name;
    // 创建事件文件
    (*NowNode).efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if ((*NowNode).efd == -1) {
        return -1;
    }
    // 添加到 epfd
    if (This.epfd != -1) {
        This.ev.events = EPOLLIN | EPOLLET; // 边缘触发
        This.ev.data.fd = (*NowNode).efd;
        if (epoll_ctl(This.epfd, EPOLL_CTL_ADD, (*NowNode).efd, &This.ev) == -1) {
            close((*NowNode).efd);
            return -1;
        }
    }
    This.EventsNumber++;
    if (This.Head == NULL) {
        Temp->next = Temp;
        Temp->prev = Temp;
        This.Head = Temp;
        NowNode = This.Head;
    } else {
        Temp->next = This.Head;
        Temp->prev = NowNode;
        NowNode->next = Temp;
        NowNode = Temp;
        This.Head->prev = Temp;
    }
    return 0;
}
// 等待某个事件（阻塞）
int _checkEvents(struct _EventGroup This, struct epoll_event *Events, uint8_t MaxEventNum) {
    while (1) {
        // 阻塞等待事件发生，-1 表示无限等待
        int nfds = epoll_wait(This.epfd, Events, MaxEventNum, -1);
        if (nfds == -1) {
            if (errno == EINTR)
                continue; // 被信号中断，重试
            perror("epoll_wait");
            break;
        }
    }
    return 0;
}
// 检查是否有事件产生
int _waitEventForName(struct _EventGroup This, strnew Name) {
    // 查找 Name 对应的 efd
    //   	while (1) {
    //        ssize_t n = read(efd, &cnt, sizeof(cnt));
    //        if (n > 0) {
    //            // 成功读到一个事件（或计数）
    //            printf("[消费者] 读取到事件计数：%lu\n", cnt);
    //        } else if (n == -1 && errno == EAGAIN) {
    //            // 无更多数据，退出循环
    //            break;
    //        } else {
    //            // 读取错误，可能是生产者线程结束并关闭 efd
    //            perror("read eventfd");
    //            running = 0;
    //            break;
    //        }
    //    }
    return 0;
}
// 初始化事件组
eventGroup newEventGroup(void) {
    eventGroup Temp = {0};
    Temp.addEvent = _addEvent;
    Temp.waitEventForName = _waitEventForName;
    Temp.checkEvents = _checkEvents;
    Temp.Head = NULL;
    // 创建epoll文件
    Temp.epfd = epoll_create1(EPOLL_CLOEXEC);
    if (Temp.epfd == -1) {
        close(Temp.epfd);
    }
    return Temp;
}

void cleanEventGroup(eventGroup This) {
    // 释放事件组链表
    if (!_THIS_MY_)
        return;
    itemevent_t *cur = This.Head;
    while (cur) {
        itemevent_t *Temp = (*cur).next;
        free(cur);
        cur = Temp;
    }
    This.Head = NULL;
    This.EventsNumber = 0;
}
