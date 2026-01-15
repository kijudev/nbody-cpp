#include <algorithm>
#include <thread>

#include "base/parallel.hpp"

namespace nbody::base {
using namespace nbody::base::type;

namespace impl {

USize get_number_of_threads() {
    const USize threads = std::thread::hardware_concurrency();
    return (threads > 0) ? threads : 1;
}

bool should_parallelize(USize length, USize min_per_thread, USize num_threads) {
    return (length >= min_per_thread) && (num_threads > 1);
}

USize calculate_thread_count(USize length, USize min_per_thread,
                             USize num_threads) {
    const USize max_threads_by_workload = length / min_per_thread;
    const USize optimal_threads =
        std::min(num_threads, max_threads_by_workload);
    return std::max(static_cast<USize>(1), optimal_threads);
}
}  // namespace impl
}  // namespace nbody::base
