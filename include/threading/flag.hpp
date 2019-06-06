#pragma once

#include <atomic>
#include <include/threading/util.hpp>

class Mutex;
class ConditionVariable;

class Flag {
    friend class Mutex;
    friend class ConditionVariable;

public:
    void Set() {
        while (true) {
            uint32_t val = data_.load();
            uint32_t new_val = SetBit(val, 0);
            if (data_.compare_exchange_weak(val, new_val)) {
                break;
            }
        }
    }

    void Unset() {
        while (true) {
            uint32_t val = data_.load();
            uint32_t new_val = UnsetBit(val, 0);
            if (data_.compare_exchange_weak(val, new_val)) {
                break;
            }
        }
    }

    bool IsSet() {
	uint32_t val = data_.load();
        return TestBit(val, 0);
    }

private:
    std::atomic<uint32_t> data_;
};
