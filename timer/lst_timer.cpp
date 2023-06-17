#include "lst_timer.h"
#include "../http/http_conn.h"

sort_timer_lst::sort_timer_lst() {
    head = NULL;
    tail = NULL;
}

sort_timer_lst::~sort_timer_lst() {
    util_timer* tmp = head;
    while (tmp) {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_lst::add_timer(util_timer* timer) {
    if (!timer) {
        return;
    } 
    if (!head) {
        head = tail = timer;
        return;
    }
    if (timer->expire < head->expire) {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    add_timer(timer, head);     // 调用的是private中的add_timer函数
}

void sort_timer_lst::adjust_timer(util_timer* timer) {
    if (!timer) {
        return;
    }
    util_timer* tmp = timer->next;
    if (!tmp || timer->expire < tmp->expire) {
        return;
    }
    if (timer == head) {
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head);
    } else {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}

void sort_timer_lst::del_timer(util_timer* timer) {
    if (!timer) {
        return;
    }
    if ((timer == head) && (timer == tail)) {
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    if (timer == head) {
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if (timer == tail) {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

// 定时任务处理函数.
// 使用统一事件源，SIGALRM信号每次被触发，主循环中调用一次定时任务处理函数，处理链表容器中到期的定时器。
void sort_timer_lst::tick() {
    if (!head) {
        return;
    }
    time_t cur = time(NULL);
    util_timer* tmp = head;
    while (tmp)
    {
       if (cur < tmp->expire) {
            break;
       }
       tmp->cb_func(tmp->user_data);
       head = tmp->next;
    }
    if (head) {
        head->prev = NULL;
    }
    delete tmp;
    tmp = head;
}

// 从头遍历找到 timer 插入的位置进行插入，保证链表的 expire 域从小到大排序
void sort_timer_lst::add_timer(util_timer* timer, util_timer* lst_head) {

}
