#pragma once

#include <vector>

#include <include/future.hpp>

class ParallelFor {
public:
    ParallelFor(size_t from, size_t to, uint8_t step_complexity,
                Executor* exec = GetGlobalExecutor())
        : from_{from}, to_{to}, exec_{exec}, step_complexity_{step_complexity} {
    }

    template <class FuncT>
    void operator()(FuncT func) const {
        std::vector<std::shared_ptr<Task>> tasks;

        size_t piece_size = (to_ - from_) / (static_cast<size_t>(step_complexity_) + 16u);
        piece_size = std::max((size_t)1, piece_size);
	tasks.reserve((to_ - from_) / piece_size);

	for (size_t begin_ix = from_; begin_ix < to_; begin_ix += piece_size) {
	    size_t end_ix = begin_ix + piece_size;
	    end_ix = std::min(end_ix, to_);
            auto work = [begin_ix, end_ix, func]() {
                for (size_t i = begin_ix; i < end_ix; ++i) {
                    func(i);
                }
                return Unit{};
            };

	    tasks.push_back(exec_->Invoke<Unit>(work));
        }

	for (auto& task: tasks) {
	    task->Wait();
	}
    }

private:
    const size_t from_, to_;
    Executor* const exec_;
    const uint8_t step_complexity_;
};
