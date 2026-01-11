#pragma once

#include <algorithm>
#include <thread>
#include <vector>

#include "type.hpp"

namespace nbody::base {
using namespace nbody::base::type;

namespace impl {
USize get_hardware_concurrency();
bool  should_parallelize(USize length, USize min_per_thread, USize num_threads);
USize calculate_thread_count(USize length, USize min_per_thread, USize num_threads);
}  // namespace impl

template <typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func func, USize min_per_thread = 1000) {
    const USize length      = static_cast<USize>(std::distance(first, last));
    const USize num_threads = impl::get_hardware_concurrency();

    if (!impl::should_parallelize(length, min_per_thread, num_threads)) {
        std::for_each(first, last, func);
        return;
    }

    const USize actual_threads = impl::calculate_thread_count(length, min_per_thread, num_threads);
    const USize chunk_size     = length / actual_threads;

    std::vector<std::thread> threads;
    threads.reserve(actual_threads);

    auto chunk_start = first;
    for (USize i = 0; i < actual_threads - 1; ++i) {
        auto chunk_end = chunk_start;
        std::advance(
            chunk_end,
            static_cast<typename std::iterator_traits<Iterator>::difference_type>(chunk_size));

        threads.emplace_back(
            [chunk_start, chunk_end, &func]() { std::for_each(chunk_start, chunk_end, func); });

        chunk_start = chunk_end;
    }

    // Last thread handles remainder
    threads.emplace_back([chunk_start, last, &func]() { std::for_each(chunk_start, last, func); });

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
}
}  // namespace nbody::base
