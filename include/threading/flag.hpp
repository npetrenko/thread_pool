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
            uint32_t val = data_.load(std::memory_order_relaxed);
            uint32_t new_val = SetBit(val, 0);
            if (data_.compare_exchange_weak(val, new_val, std::memory_order_relaxed)) {
                break;
            }
        }
    }

    void Unset() {
        while (true) {
            uint32_t val = data_.load();
            uint32_t new_val = UnsetBit(val, 0);
            if (data_.compare_exchange_weak(val, new_val, std::memory_order_relaxed)) {
                break;
            }
        }
    }

    bool IsSet() {
	uint32_t val = data_.load(std::memory_order_relaxed);
        return TestBit(val, 0);
    }

private:
    std::atomic<uint32_t> data_{0};
};
