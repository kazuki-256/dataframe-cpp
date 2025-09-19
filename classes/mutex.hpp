#pragma once

#include <list>
#include <chrono>
#include <thread>


struct df_mutex_t {
    long start = 0, end = -1;
    int use_count = 0;
    bool is_write = false;
};



class df_lock_t {
protected:
    df_mutex_t* _mutex = NULL;

    void _basic_lock(std::list<df_mutex_t>& mutexs, long start, long end, bool is_write) {
        // == reuse old ptr ==
        auto new_mutex = mutexs.begin();
        for (; new_mutex != mutexs.end(); new_mutex++) {
            if (new_mutex->use_count == 0) {
                mutexs.splice(mutexs.end(), mutexs, new_mutex);
                break;
            }
        }

        // == setup mutex ==
        if (new_mutex == mutexs.end()) {
            mutexs.emplace_back(start, end, 1, false);
        }
        else {
            *new_mutex = {start, end, 1, false};
        }
        
        // == wait for unquire_lock ==
        for (auto iter = mutexs.begin(); iter != new_mutex; iter++) {
            while ((is_write || (is_write == false && iter->is_write)) && iter->use_count > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }


public:
    ~df_lock_t() {
        if (_mutex) {
            _mutex->use_count--;
        }
    }

    constexpr df_lock_t() {}

    df_lock_t(std::list<df_mutex_t>& mutexs, long start, long end, bool is_write) {
        _basic_lock(mutexs, start, end, is_write);
    }

    void lock(std::list<df_mutex_t>& mutexs, long start, long end, bool is_write) {
        _basic_lock(mutexs, start, end, is_write);
    }
};
