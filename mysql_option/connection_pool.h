#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include "../synchronize/lock.h"
#include "../synchronize/signal.h"
#include "../log/log.h"

using namespace std;

class connection_pool {
private:
    static connection_pool* get_instance();      // single model

    MYSQL* GetConnection();     // get one connection from pool
    bool ReleaseConnection(MYSQL* conn);       // release connection
    int CountFreeConnection();        // get free connection
    void DestroyPool();         // destory connection pool

    void init(string url, string User, string PassWord, string DatabaseName, int Port, int MaxConn, int close_log);

public:
    connection_pool();
    ~connection_pool();

    int m_MaxConn;
    int m_CurrConn;
    int FreeConn;
    lock m_lock;
    list<MYSQL*> connList;      // connection pool
    signal m_connection;
    string m_url;			 //主机地址
	string m_Port;		 //数据库端口号
	string m_User;		 //登陆数据库用户名
	string m_PassWord;	 //登陆数据库密码
	string m_DatabaseName; //使用数据库名
	int m_close_log;	//日志开关

};

// 将数据库连接的获取与释放通过RAII机制封装，避免手动释放。
// RAII机制：自动进行资源的获取和释放
class ConnectionRAII{
private:
    MYSQL* conRALL;
    connection_pool* poolRALL;
public:
    ConnectionRAII(MYSQL** conn, connection_pool* connPool);
    ~ConnectionRAII();

};

#endif