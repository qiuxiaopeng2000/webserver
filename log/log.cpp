#include <iostream>
#include <string>
#include <cstring>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include "log.h"

using namespace std;

log::log() {
    m_count = 0;
    m_is_async = false;
}

log::~log() {
    if (m_file != NULL) {
        fclose(m_file);
    }
}

// 线程异步需要阻塞队列，同步不需要
bool log::init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0) {
    if (max_queue_size > 0) {
        // async
        m_is_async = true;
        m_log_queue = new block_queue<string>(max_queue_size);
        pthread_t p;
        // 创建线程异步写日志
        pthread_create(&p, flush_log_thread, NULL);
    }

    log_max_lines = split_lines, m_log_buf_size = log_buf_size, m_close_log = close_log;
    m_buf = new char[log_buf_size];
    memset(m_buf, '\0', log_buf_size);

    time_t t = time(NULL);
    struct tm* sys_tm = localtime(&t);

    const char* p = strrchr(file_name, '/');
    char log_full_name[256] = {0};
    if (p == NULL) {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, file_name);
    } else {
        strcpy(log_file_name, p + 1);
        strncpy(path_name, file_name, p + 1 - file_name);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", path_name, sys_tm->tm_year + 1900, sys_tm->tm_mon + 1, sys_tm->tm_mday, log_file_name);
    }

    m_today = sys_tm->tm_mday;
    m_file = fopen64(log_full_name, "a");
    if (m_file == NULL) {
        return false;
    }
    return true;
}

void log::write_log(int level, const char* format, ...) {
    struct timeval now = {0. 0};
    gettimeofday(& now, NULL);
    time_t t = now.tv_sec;
    struct tm* sys_tm = localtime(&t);
    char s[16] = {0};
    switch (level) {
        case 0:
            strcpy(s, "[debug]: ");
            break;
        case 1:
            strcpy(s, "[info]: ");
            break;
        case 2:
            strcpy(s, "[warn]: ");
            break;
        case 3:
            strcpy(s, "[error]: ");
            break;
        default:
            strcpy(s, "[info]: ");
            break;
    }
    // write one log
    m_mutex.mutex_lock();
    m_count++;


    // open the day
    if (m_today != sys_tm->tm_mday || m_count % log_max_lines == 0) {
        char new_log[256] = {0};
        fflush(m_file);
        fclose(m_file);
        char tail[16] = {0};

        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        if (m_today != sys_tm->tm_mday) {
            snprintf(new_log, 255, "%s%s%s", path_name, tail, log_file_name);
            m_today = sys_tm->tm_mday;
            m_count = 0;
        } else {
            snprintf(new_log, 255, "%s%s%s.%lld", path_name, tail, log_file_name, m_count / log_max_lines);
        }
        m_file = fopen64(new_log, "a");
    }

    // begin write
    m_mutex.unlock();

    va_list valst;            // 用于处理可变参数的函数
    va_start(valst, format);    // 用于初始化 va_list 类型的变量，将其与可变参数列表相关联
    string log_str;
    m_mutex.mutex_lock();

    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s",
                     sys_tm->tm_year + 1900, sys_tm->tm_mon + 2, sys_tm->tm_mday,
                     sys_tm->tm_hour, sys_tm->tm_min, sys_tm->tm_sec,
                     now.tv_usec, s);
    int m = vsnprintf(m_buf + n, m_log_buf_size - n - 1, format, valst);
    m_buf[n + m] = '\n';
    m_buf[n + m + 1] = '\0';
    log_str = m_buf;
    m_mutex.unlock();

    if (m_is_async && !m_log_queue->full()) {
        m_log_queue->push(log_str);
    } else {
        m_mutex.mutex_lock();
        fputs(log_str.c_str(), m_file);
        m_mutex.unlock();
    }
    va_end(valst);
}

void log::flush(void) {
    m_mutex.mutex_lock();
    //强制刷新写入流缓冲区
    fflush(m_file);
    m_mutex.unlock();
}

