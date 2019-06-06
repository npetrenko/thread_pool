#pragma once

#include <memory>
#include <type_traits>

struct Unit {
};

class Task;

template <class T, class FuncT>
class Future;

template <class T, class FuncT>
using FuturePtr = std::shared_ptr<Future<T, FuncT>>;

class Executor {
public:
    virtual ~Executor() = default;

    virtual void Submit(std::shared_ptr<Task> task) = 0;

    virtual void StartShutdown() = 0;
    virtual void WaitShutdown() = 0;

    template <class T, class FuncT>
    FuturePtr<T, std::decay_t<FuncT>> Invoke(FuncT&& fn);

protected:
    inline static void SetCompleted(Task* task);
    inline static void SetException(Task* task, std::exception_ptr exc);
};

std::shared_ptr<Executor> MakeThreadPoolExecutor(int num_threads);
Executor* GetGlobalExecutor();
