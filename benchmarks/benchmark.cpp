#include <benchmark/benchmark.h>
#include <include/for_loop.hpp>

template <bool is_parallel>
static void SumBenchmark(benchmark::State& state) {
    std::vector<uint32_t> counters(state.range(), 0);
    for (size_t i = 0; i < 10; ++i) {
        ParallelFor{0, counters.size(), 1}([&](size_t ix) { ++counters[ix]; });
    }

    while (state.KeepRunning()) {
	if constexpr(is_parallel) {
            ParallelFor{0, counters.size(), 1}([&](size_t ix) { ++counters[ix]; });
        } else {
	    for (auto& elem: counters) {
		++elem;
	    }
	}
        benchmark::DoNotOptimize(counters[0]);
    }
}

BENCHMARK_TEMPLATE(SumBenchmark, 0)->Range(1024, 1024*1024);
BENCHMARK_TEMPLATE(SumBenchmark, 1)->Range(1024, 1024*1024);
BENCHMARK_MAIN();
