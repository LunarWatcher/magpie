#pragma once

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <atomic>
#include <cstdint>

namespace magpie::internals {

struct Worker {
    asio::io_context ioContext;
    asio::executor_work_guard<decltype(ioContext)::executor_type> workGuard;
    std::atomic<uint32_t> workload = 0;

    Worker() : workGuard(asio::make_work_guard(ioContext)) {}
    Worker(Worker&) = delete;
    Worker(Worker&&) = delete;
};

}
