#ifndef SIGNAL_H
#define SIGNAL_H

#include <semaphore.h>
#include <exception>

class signal {
private:
    sem_t sem;

public:
    signal() {
        if (sem_init(& sem, 0, 0)) {
            throw std::exception();
        }
    }
    signal(int val) {
        if (sem_init(& sem, 0, val)) {
            throw std::exception();
        }
    }
    ~signal() {
        if (sem_destroy(& sem)) {
            throw std::exception();
        }
    }
    bool wait() {
        return sem_wait(& sem) == 0;
    }
    bool post() {
        return sem_post(& sem) == 0;
    }
}; 

#endif