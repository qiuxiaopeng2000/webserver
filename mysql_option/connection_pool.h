#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include "../synchronize/lock.h"
#include "../synchronize/signal.h"
#include "../log/log.h"

using namespace std;

class connction_pool {
private:
    static connction_pool* get_instance();      // single model

    MYSQL* GetConnection();     // create connection
    bool ReleaseConnection(MYSQL* conn);       // release connection
    int GetFreeConnection();        // get free connection
    void DestroyPool();         // destory connection pool

    void init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log);

public:
    connction_pool();
    ~connction_pool();

    int m_MaxConn;
    int m_CurrConn;
    int FreeConn;
    lock m_lock;
    list<MYSQL*> connList;      // connection pool
    signal m_connection;
    int m_close_log;        // 日志开关

};

class ConnectionRAII{
private:
    MYSQL* conRALL;
    connction_pool* pollRALL;
public:
    ConnectionRAII(MYSQL** conn, connction_pool* connPool);
    ~ConnectionRAII();

};

#endif