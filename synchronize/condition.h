#ifndef COND_H      // 确保头文件只被包含一次，避免重复定义和编译错误。
#define COND_H

#include <pthread.h>
#include <exception>

class cond {
private:
    pthread_cond_t m_cond;
public:
    cond() {
        if (pthread_cond_init(& m_cond, NULL)) {
            throw std::exception();
        }
    }
    ~cond() {
        if (pthread_cond_destroy(& m_cond)) {
            throw std::exception();
        }
    }
    bool wait(pthread_mutex_t* mutex) {
        int status = 0;
        pthread_mutex_lock(mutex);
        status = pthread_cond_wait(& m_cond, mutex);
        pthread_mutex_unlock(mutex);
        return status == 0;
    }
    bool wait(pthread_mutex_t* mutex, struct timespec* t) {
        int status = 0;
        pthread_mutex_lock(mutex);
        status = pthread_cond_timedwait(& m_cond, mutex, t);
        pthread_mutex_unlock(mutex);
        return status == 0;
    }
    bool signal() {
        return pthread_cond_signal(& m_cond) == 0;
    }
    bool broadcast() {
        return pthread_cond_broadcast(& m_cond) == 0;
    }
};

#endif