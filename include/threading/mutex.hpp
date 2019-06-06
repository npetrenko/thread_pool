#pragma once

#include <thread>

#include <include/threading/flag.hpp>
#include <include/threading/util.hpp>

class Mutex {
public:
    Mutex(Flag* flag) : flag_{flag} {
    }

    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex& operator=(Mutex&&) = delete;

    void Lock() const {
	while (true) {
	    uint32_t val = flag_->data_.load();
	    if (TestBit(val, 1)) {
		std::this_thread::yield();
		continue;
	    }
	    uint32_t new_val = SetBit(val, 1);
	    if (flag_->data_.compare_exchange_weak(val, new_val)) {
		break;
	    }
	}
    }

    void Unlock() const {
	while (true) {
	    uint32_t val = flag_->data_.load();
	    uint32_t new_val = UnsetBit(val, 1);
	    if (flag_->data_.compare_exchange_weak(val, new_val)) {
		break;
	    }
	}
    }

private:
    Flag* const flag_;
};
