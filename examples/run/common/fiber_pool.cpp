// Local include(s).
#include "fiber_pool.hpp"

// Boost.Fiber include(s).
#include <boost/fiber/algo/shared_work.hpp>
#include <boost/fiber/fiber.hpp>

// System include(s).
#include <thread>

namespace traccc {
fiber_pool::fiber_pool(int nFibers) {
    m_threads.reserve(static_cast<std::size_t>(nFibers));
    /// initialize fiber scheduler for sharing fiber queues with workers
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>(
        true);  /// suspend fiber scheduler on the calling thread
    for (int i = 0; i < nFibers; ++i) {
        m_threads.emplace_back([this]() {
            /// initialize fiber scheduler for sharing fiber queues
            boost::fibers::use_scheduling_algorithm<
                boost::fibers::algo::shared_work>(
                false);  /// don't suspend fiber scheduler on worker threads
            std::unique_lock<boost::fibers::mutex> lock(
                m_mutex);  /// suspend main fiber until destruction
            m_cv.wait(lock);
        });
        m_thread_indices[m_threads.at(static_cast<std::size_t>(i)).get_id()] =
            i;
    }
}

fiber_pool::~fiber_pool() {
    m_cv.notify_all();
}

int fiber_pool::get_thread_index(std::thread::id id) const {
    return m_thread_indices.at(id);
}
}  // namespace traccc
