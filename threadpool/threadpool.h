#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../synchronize/lock.h"
#include "../synchronize/signal.h"
#include "../mysql_option/connection_pool.h"

template<typename T>
class threadpool {
private:
    int m_thread_number;
    int m_max_request;
    pthread_t* m_threads;
    std::list<T*> m_request_queue;  // request queue
    lock m_lock;
    signal m_signal;
    connection_pool* m_conPool;
    int m_actor_model;  // 模型切换

    static void* worker(void* arg);     /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    void run();

public:
    threadpool(int actor_model, connection_pool* conPool, int thread_number = 8, int max_request = 10000);
    ~threadpool();
    bool append(T* request, int state);
    bool append_p(T* request);
}

template<typename T>
threadpool<T>::threadpool(int actor_model, connection_pool* conPool, int thread_number = 8, int max_request = 10000) {
    if (thread_number <= 0 || max_request <= 0)
        throw std::exception();
    m_thread_number = thread_number;
    m_max_request = max_request;
    m_threads = new pthread_t[m_thread_number];
    for (int i = 0; i < m_thread_number; ++i) {
        pthread_create(m_threads[i], NULL, worker, this);
        pthread_detach(m_threads[i]);       // 用于将一个线程设置为分离状态，分离状态的线程在结束时会自动释放其资源，无需其他线程调用 
    }
}

template<typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
}

template<typename T>
bool threadpool<T>::append(T* request, int state) {
    m_lock.mutex_lock();
    if (m_request_queue.size() >= m_max_request) {
        m_lock.unlock();
        return false;
    }
    request->m_state  = state;
    m_request_queue.push_back(request);
    m_lock.unlock();
    m_signal.post();
    return true;
}

template<typename T>
bool threadpool<T>::append(T* request) {
    m_lock.mutex_lock();
    if (m_request_queue.size() >= m_max_request) {
        m_lock.unlock();
        return false;
    }
    m_request_queue.push_back(request);
    m_lock.unlock();
    m_signal.post();
    return true;
}

template<typename T>
void* threadpool::worker(void* arg) {
    threadpool* pool = (threadpool*) arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run() {
    while (true) {
        m_signal.wait();
        m_lock.mutex_lock();
        if (m_request_queue.empty()) {
            m_lock.unlock();
            continue;
        }
        T* request = m_request_queue.front();
        m_request_queue.pop_front();
        m_lock.unlock();
        if (!request)
            continue;
        if (m_actor_model == 1) {
            if (request->m_state == 0) {
                if (request->read_once()) {
                    request->imporv = 1;
                    ConnectionRAII mysqlcon(&request->mysql, m_conPool);
                    request->process();
                } else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            } else {
                if (request->write()) {
                    request->improv = 1;
                } else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        } else {
            ConnectionRAII mysqlcon(&request->mysql, m_conPool);
            request->process();
        }
    }
}

#endif