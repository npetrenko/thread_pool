#pragma once

#include <thread>

#include <include/threading/flag.hpp>
#include <include/threading/mutex.hpp>
#include <include/threading/util.hpp>

class ConditionVariable {
    void Wait(Flag* flag) const {
	while (true) {
	    uint32_t val = flag->data_.load();
	    if (!TestBit(val, 0)) {
		uint32_t new_val = UnsetBit(val, 1);
                if (flag->data_.compare_exchange_weak(val, new_val)) {
                    std::this_thread::yield();
                    Mutex mut{flag};
                    mut.Lock();
		    continue;
                } else {
                    continue;
                }
            }
	    uint32_t new_val = SetBit(val, 1);
	    if (flag->data_.compare_exchange_weak(val, new_val)) {
		break;
	    }
	}
    }
};
