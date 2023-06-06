#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>
#include <exception>

class lock {
private:
    pthread_mutex_t mutex;

public:
    lock() {
        if (pthread_mutex_init(& mutex, NULL)) {
            throw std::exception();
        }
    }
    ~lock() {
        if (pthread_mutex_destroy(& mutex)) {
            throw std::exception();
        }
    }
    int mutex_lock() {
        return pthread_mutex_lock(& mutex);
    }
    int unlock() {
        return pthread_mutex_unlock(& mutex);
    }
    pthread_mutex_t* get_lock() {
        return & mutex;
    }
};

#endif