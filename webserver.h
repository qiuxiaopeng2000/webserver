#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "./threadpool/threadpool.h"
#include "./http/http_conn.h"

const int MAX_FD = 65536;   // 最大文件描述符
const int MAX_EVENT_NUMBER = 10000;     // 最大事件数
const int TIMESLOT = 5;     // 最大超时单位

class WebServer {
private:

public:
    // 基础
    int m_port;
    char* m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    int m_pipfd[2];
    int m_epollfd;
    http_conn* users;

    // 数据库相关
    connection_pool* m_connPool;
    string m_user;      // 数据库用户名
    string m_password;      // 数据库密码
    string m_databasename;      // 数据库名
    int m_sql_m;

    // 线程池相关
    threadpool<http_conn> *m_pool;
    int m_thread_num;

    // epoll_event 相关
    epoll_event events[MAX_EVENT_NUMBER];
    int m_listenfd;
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_CONNTrigmode;

    // 定时器相关
    client_data* users_timer;
    Utils utils;

public:
    WebServer();
    ~WebServer();

    void init(int port, string user, string password, string databasename,
              int log_write, int opt_linger, int trigmode, int sql_num,
              int thread_num, int close_log, int actor_model);
    
    void thread_pool();
    void sql_pool();
    void log_write();
    void trig_mode();
    void eventListen();
    void eventLoop();
    void timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(util_timer* timer);
    void deal_timer(util_timer* timer, int sockfd);
    void dealclientdata();
    bool dealwithsignal(bool& timeout, bool& stop_server);
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);
};

#endif