#pragma once

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <atomic>
#include <cstdint>

namespace magpie::internals {

struct Worker {
    asio::io_context context;
    asio::executor_work_guard<decltype(context)::executor_type> workGuard;
    std::atomic<uint32_t> workload = 0;

    Worker() : workGuard(asio::make_work_guard(context)) {}
    Worker(Worker&) = delete;
    Worker(Worker&&) = delete;
};

}
