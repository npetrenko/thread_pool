#pragma once

#include <atomic>

class Mutex;
class ConditionVariable;

class Flag {
    friend class Mutex;
    friend class ConditionVariable;

public:
    void Set() {
        while (true) {
            uint32_t val = data_.load();
            uint32_t new_val = val | 1;
            if (data_.compare_exchange_weak(val, new_val)) {
                break;
            }
        }
    }

    void Unset() {
        while (true) {
            uint32_t val = data_.load();
            uint32_t new_val = val & ~((uint32_t)1);
            if (data_.compare_exchange_weak(val, new_val)) {
                break;
            }
        }
    }

    bool IsSet() {
        return data_.load() & (uint32_t)1;
    }

private:
    std::atomic<uint32_t> data_;
};
