#pragma once

#include <thread>

#include <include/threading/flag.hpp>
#include <include/threading/mutex.hpp>

class ConditionVariable {
    void Wait(Flag* flag) {
	while (true) {
	    uint32_t val = flag->data_.load();
	    if (!(val & 1)) {
		uint32_t new_val = val & ~((uint32_t)2);
                if (flag->data_.compare_exchange_weak(val, new_val)) {
                    std::this_thread::yield();
                    Mutex mut{flag};
                    mut.Lock();
		    continue;
                } else {
                    continue;
                }
            }
	    uint32_t new_val = val | 2;
	    if (flag->data_.compare_exchange_weak(val, new_val)) {
		break;
	    }
	}
    }
};
