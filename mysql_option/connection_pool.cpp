// #include <string.h>
#include <string>
// #include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "connection_pool.h"

using namespace std;

connection_pool* connection_pool::get_instance() {
    static connection_pool* conn;
    return conn;
}

connection_pool::init(string url, string User, string PassWord, string DatabaseName, int Port, int MaxConn, int close_log) {
    m_url = url;
    m_Port = Port;
    m_User = User;
    m_PassWord = PassWord;
    m_DatabaseName = DatabaseName;
    m_close_log = close_log;

    for (int i = 0; i < MaxConn; i++) {
        MYSQL* con = mysql_init(NULL);
        if (con == NULL) {
            LOG_ERROR("MYSQL ERROR");
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DatabaseName.c_str(), Port, NULL, 0);

        if (con == NULL) {
            LOG_ERROR("MYSQL ERROR");
            exit(1);
        }
        connList.push_back(con);
        FreeConn++;
    }
    m_connection = signal(FreeConn);
    m_MaxConn = FreeConn;
}

//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL* connection_pool::GetConnection() {
    if (FreeConn == 0)
        return false;

    MYSQL* con = NULL;
    m_connection.wait();    
    m_lock.mutex_lock();        // 使用互斥锁处理连接池
    con = connList.front();
    connList.pop_front();
    FreeConn--;
    m_CurrConn++;
    m_lock.unlock();
    return con;
}

bool connection_pool::ReleaseConnection(MYSQL* con) {
    if (m_CurrConn == 0 || con == NULL)
        returm false;

    m_connection.post();
    m_lock.mutex_lock();
    connList.push_back(con);
    FreeConn++;
    m_CurrConn--;
    m_lock.unlock();
    return true;
}

int connection_pool::CountFreeConnection() {
    return FreeConn;
}

void connection_pool::DestoryPool() {
    m_lock.mutex_lock();

    if (connList.size() > 0) {
        for (auto it = connList.begin(); it != connList.end(); it++) {
            MYSQL* con = * it;
            mysql_close(con);
        }
        m_CurrConn = 0;
        FreeConn = 0;

        connList.close();
    }
    m_lock.unlock();
}

ConnectionRAII::ConnectionRAII(MYSQL** con, connection_pool* conPool) {
    *con = conPool->GetConnection();

    conRALL = con;
    poolRALL = conPol;
}

ConnectionRAII::~ConnectionRAII() {
    poolRALL->ReleaseConnection(conRALL);
}

