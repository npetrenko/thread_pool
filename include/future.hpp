#include <memory>

#include <include/executors.hpp>
#include <include/task.hpp>

template <class T, class FuncT>
class Future : public Task {
public:
    friend class Executor;

    explicit Future(const FuncT& func) : func_{func} {
    }

    explicit Future(FuncT&& func) : func_{std::move(func)} {
    }

    void Run() override {
        val_ = func_();
    }

    T Get() & {
        GetHelper();
        return val_;
    }

    T Get() && {
        GetHelper();
        return std::move(val_);
    }

private:
    void GetHelper() {
        Wait();
        if (IsFailed()) {
            std::rethrow_exception(GetError());
        }
    }
    T val_;
    FuncT func_;
};

template <class T, class FuncT>
FuturePtr<T, std::decay_t<FuncT>> Executor::Invoke(FuncT&& fn) {
    auto task = std::make_shared<Future<T, std::decay_t<FuncT>>>(std::forward<FuncT>(fn));
    Submit(task);
    return task;
}
