#include <include/executors.hpp>
#include <include/task.hpp>
#include <include/exceptions.hpp>

#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <exception>
#include <cassert>
#include <optional>
#include <vector>

#include <include/threading/cond_var.hpp>

struct OnTaskFinishCallback {
    Task* task;
    void operator()(std::exception_ptr eptr) const;
};

class ThreadPool {
public:
    explicit ThreadPool(int num_threads);

    bool Run(std::shared_ptr<Task> fun, std::optional<OnTaskFinishCallback> on_execution = std::nullopt);

    void StartShutdown();
    void WaitShutdown();

    ~ThreadPool();

private:
    using ElemT = std::pair<std::shared_ptr<Task>, std::optional<OnTaskFinishCallback>>;
    void PoolLoop();
    void ProcessSingleTask(ElemT&& pool_element);

    Flag threads_have_work_;

    Mutex mut_{&threads_have_work_};
    ConditionVariable pool_cv_;

    std::deque<ElemT> pool_;
    std::vector<std::thread> threads_;
    bool stopped_{false};
    std::atomic<bool> has_waited_for_shutdown_{false};
};

class ThreadPoolExecutor : public Executor {
    friend struct OnTaskFinishCallback;

public:
    ThreadPoolExecutor(int num_threads) : thread_pool_(num_threads) {
    }

    void Submit(std::shared_ptr<Task> task) override {
        OnTaskFinishCallback callback{task.get()};
        if (!thread_pool_.Run(std::move(task), std::move(callback))) {
            throw ThreadPoolExeption{};
        }
    }

    void StartShutdown() override {
        thread_pool_.StartShutdown();
    }

    void WaitShutdown() override {
        thread_pool_.WaitShutdown();
    }

private:
    ThreadPool thread_pool_;
};

void OnTaskFinishCallback::operator()(std::exception_ptr eptr) const {
    if (eptr) {
        ThreadPoolExecutor::SetException(task, eptr);
    } else {
        ThreadPoolExecutor::SetCompleted(task);
    }
}

std::shared_ptr<Executor> MakeThreadPoolExecutor(int num_threads) {
    return std::make_shared<ThreadPoolExecutor>(num_threads);
}

Executor* GetGlobalExecutor() {
    static auto executor{std::make_unique<ThreadPoolExecutor>(std::thread::hardware_concurrency())};
    return executor.get();
}

ThreadPool::ThreadPool(int num_threads) {
    assert(num_threads > 0);
    for (int i = 0; i < num_threads; ++i) {
        threads_.emplace_back([this] { this->PoolLoop(); });
    }
}

bool ThreadPool::Run(std::shared_ptr<Task> fun, std::optional<OnTaskFinishCallback> on_execution) {
    {
        std::lock_guard lock(mut_);
        if (stopped_) {
            return false;
        }
        pool_.emplace_back(std::move(fun), std::move(on_execution));
        threads_have_work_.Set();
    }
    // pool_cv_.notify_one();
    return true;
}

void ThreadPool::StartShutdown() {
    {
        std::lock_guard lock(mut_);
        stopped_ = true;
        threads_have_work_.Set();
    }
    // pool_cv_.notify_all();
}

void ThreadPool::WaitShutdown() {
    if (!has_waited_for_shutdown_.exchange(true)) {
        for (auto& th : threads_) {
            th.join();
        }
    }
}

ThreadPool::~ThreadPool() {
    if (!has_waited_for_shutdown_.load()) {
        StartShutdown();
        WaitShutdown();
    }
}

void ThreadPool::PoolLoop() {
    while (true) {
        ElemT current_execution;
        bool needs_notify;
        {
            std::unique_lock lock(mut_);
            if (pool_.empty() && !stopped_) {
                // pool_cv_.wait(lock, [&] { return !(pool_.empty() && !stopped_); });
		pool_cv_.Wait(&threads_have_work_);
            }
            if (stopped_ && pool_.empty()) {
                return;
            }
	    assert(!pool_.empty());

            current_execution = std::move(pool_.front());
            pool_.pop_front();
            needs_notify = !pool_.empty() || stopped_;
	    if (!needs_notify) {
		threads_have_work_.Unset();
	    }
        }
        if (needs_notify) {
            // pool_cv_.notify_one();
        }
        ProcessSingleTask(std::move(current_execution));
    }
}

void ThreadPool::ProcessSingleTask(ElemT&& pool_element) {
    try {
        pool_element.first->Run();
        if (pool_element.second) {
            pool_element.second.value()(nullptr);
        }
    } catch (...) {
        if (pool_element.second) {
            pool_element.second.value()(std::current_exception());
        } else {
            throw;
        }
    }
}
