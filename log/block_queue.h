#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <pthread.h>
#include <sys/time.h>
#include "../webserver/synchronize/condition.h"
#include "../webserver/synchronize/lock.h"

template <class T>
class block_queue {
private:
    // synchronize
    lock m_mutex;
    cond m_cond;

    // block queue
    int max_size;
    int m_size;
    int m_front;
    int m_back;
    T* m_queue;

public:
    block_queue(int size) {
        if (size <= 0) {
            exit(-1);
        }

        max_size = size;
        front = back = -1;
        m_size = 0;
        m_queue = new T[max_size];
    }
    ~block_queue() {
        m_mutex.mutex_lock();
        if (m_queue != NULL)
            delete [] m_queue;
        m_mutex.unlock();
    }
    void clear() {
        m_mutex.mutex_lock();
        m_size = 0;
        back = front = -1;
        m_mutex.unlock();
    }
    bool full() {
        m_mutex.mutex_lock();
        if (m_size == max_size) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    bool empty() {
        m_mutex.mutex_lock();
        if (m_size == 0) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    int size() {
        int temp = 0;
        m_mutex.mutex_lock();
        temp = m_size;
        m_mutex.unlock();
        return temp;
    }
    int max_size() {
        int temp = 0;
        m_mutex.mutex_lock();
        temp = max_size;
        m_mutex.unlock();
        return temp;
    }
    // need consider if the queue is full
    bool push(const T& val) {
        m_mutex.mutex_lock();
        if (m_size >= max_size) {
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }
        m_front = (m_front + 1) % max_size;
        m_size++;
        m_queue[m_front] = val;

        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }
    // need consider if the queue is empty
    bool pop(T& val) {
        m_mutex.mutex_lock();
        if (m_size <= 0) {
            if (!m_cond.wait(m_mutex.get_lock())) {
                m_mutex.unlock();
                return false;
            }
        }
        m_back = (m_back + 1) % max_size;
        val = m_queue[m_back];
        m_size--;
        m_mutex.unlock();
        return true;
    }
    // timeout proccess
    bool pop(T& val, int ms_time) {
        struct timespec t = {0, 0};
        struct timespec now = {0, 0};
        gettimeofday(& now, NULL);
        m_mutex.mutex_lock();
        if (m_size <= 0) {
            t.tv_sec = now.tv_sec + time / 1000;
            t.tv_nsec = (time % 1000) * 1000;
            if (!m_cond.wait(m_mutex.get_lock(), & t)) {
                m_mutex.unlock();
                return false;
            }
            if (m_size < 0) {
                m_mutex.unlock();
                return false;
            }
        }
        m_back = (m_back + 1) % max_size;
        val = m_queue[m_back];
        m_size--;
        m_mutex.unlock();
        return true;
    }
};


#endif