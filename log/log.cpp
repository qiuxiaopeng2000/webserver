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
log::init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0) {
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



