#ifndef LOG_H
#define LOG_H

#include "../webserver/log/block_queue.h"
#include <iostream>
#include <string>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

class log {
private:
    static log* instance;
    FILE* m_file;
    char path_name[128];     // path name
    char log_file_name[128];    // file name
    int log_max_lines;          // log lines
    int m_log_buf_size;     // log buf size
    long long m_count;      // log total lines which include history log
    int m_today;            // the date when write the log

    block_queue<string> m_log_queue;        // block queue for log
    char* m_buf;
    lock m_mutex;
    bool m_is_async;        // the flag for log async
    int m_close_log;        // flag for log file close

    log();
    virtual ~log();
    void* async_write_log() {       // void * 表示通用类型指针，用于不知道需要返回不同类型指针的情况，或者在不确定具体类型时需要传递指针的情况。
        string single_log;
        while (m_log_queue.pop(single_log)) {
            m_mutex.mutex_lock();
            fputs(single_log.c_str(), m_file);
            m_mutex.unlock();
        }
    }
public:
    static log* get_instance() {
        if (instance == nullptr)
            log::log();
        return instance;
    }
    static void* flush_log_thread(void* args) {
        log::get_instance()->async_write_log();
    } 
    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);
};

#define LOG_DEBUG(format, ...) if (0 == m_close_log) {
    log::get_instance()->write_log(0, format, ##__VA_ARGS__);
    log::get_instance()->flush();
}
#define LOG_INFO(format, ...) if (0 == m_close_log) {
    log::get_instance()->write_log(1, format, ##__VA_ARGS__);
    log::get_instance()->flush();
}
#define LOG_WARN(format, ...) if (0 == m_close_log) {
    log::get_instance()->write_log(2, format, ##__VA_ARGS__);
    log::get_instance()->flush();
}
#define LOG_ERROR(format, ...) if (0 == m_close_log) {
    log::get_instance()->write_log(3, format, ##__VA_ARGS__);
    log::get_instance()->flush();
}

#endif