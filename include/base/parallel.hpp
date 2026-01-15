// ==============================================================================
// parallel.hpp
// Parallel iteration helpers and thread utilities for the nbody project.
// Provides utilities for running loops in parallel using std::thread.
// ==============================================================================

#pragma once

#include <algorithm>
#include <thread>
#include <vector>

#include "type.hpp"

namespace nbody::base {
using namespace nbody::base::type;

namespace impl {
// Returns the number of available hardware threads.
USize get_number_of_threads();

// Decides if the loop should be parallelized based on the size of the
// container.
bool should_parallelize(USize length, USize min_per_thread, USize num_threads);

// Returns the optimal number of threads that should be used in the loop
// paralallization.
USize calculate_thread_count(USize length, USize min_per_thread,
                             USize num_threads);
}  // namespace impl

// Parallel iterator helper. Runs the callback function on the provided
// iterator in pararell.
// WARNING: The operation will not always be run in parallel.
template <typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func func,
                       USize min_per_thread = 1000) {
    const USize length      = static_cast<USize>(std::distance(first, last));
    const USize num_threads = impl::get_number_of_threads();

    if (!impl::should_parallelize(length, min_per_thread, num_threads)) {
        std::for_each(first, last, func);
        return;
    }

    const USize actual_threads =
        impl::calculate_thread_count(length, min_per_thread, num_threads);
    const USize chunk_size = length / actual_threads;

    std::vector<std::thread> threads;
    threads.reserve(actual_threads);

    auto chunk_start = first;
    for (USize i = 0; i < actual_threads - 1; ++i) {
        auto chunk_end = chunk_start;
        std::advance(
            chunk_end,
            static_cast<
                typename std::iterator_traits<Iterator>::difference_type>(
                chunk_size));

        threads.emplace_back([chunk_start, chunk_end, &func]() {
            std::for_each(chunk_start, chunk_end, func);
        });

        chunk_start = chunk_end;
    }

    threads.emplace_back([chunk_start, last, &func]() {
        std::for_each(chunk_start, last, func);
    });

    for (auto& thread : threads) {
        thread.join();
    }
}
}  // namespace nbody::base
