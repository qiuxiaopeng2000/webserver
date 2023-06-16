#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <time.h>

#include "log/log.h"

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer* timer;
};

class util_timer {
private:

public:
    util_timer* prev;
    util_timer* next;
    client_data* user_data;
    time_t expire;

public:
    util_timer(): prev(NULL), next(NULL) {}     // 只在初始化函数中使用，用于初始化成员变量的值
    void (* cb_func)(client_data*);         // 函数指针

}

class sort_timer_lst {      // 维护定时器链表
private:
    util_timer* head;
    util_timer* tail;

private:
    void add_timer(util_timer* timer, util_timer* lst_head);

public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer* timer);
    void adjust_timer(util_timer* timer);
    void del_timer(util_timer* timer);
    void tick();
}

class Utils {
private:
      
public:
    static int* u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;

public:
    Utils();
    ~Utils();

    void init(int timeslot);

    // 对文件描述符设置为非阻塞
    int setnonblocking(int fd);
    /*  1. 将一个文件描述符（通常是套接字）注册到内核的事件表中，并指定关注的事件类型为读事件. 这样，当该文件描述符上有可读数据时，内核会通知应用程序。
        2. 同时将epoll的工作模式设置为ET边缘触发模式，在ET模式下，只有在状态发生变化时才会通知应用程序.
        3. 开启EPOLLONESHOT，EPOLLONESHOT是epoll事件注册时的一个选项，用于确保一个注册的事件只被一个线程处理。
    */
   void addwritefd(int epollfd, int fd, bool one_shot, int TRIGModel);

   // signal处理函数
   static void sig_handler(int sig);

   // set signal
   void addsig(int sig, void(handler)(int), bool restart = true);

   // 定时处理任务，重新定时以不断触发SIGALRM信号
   void timer_handler();

   void show_eroor(int connfd, const char* info);
}

void cb_func(client_data* user_data);
#endif