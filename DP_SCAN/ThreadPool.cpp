#include "ThreadPool.h"

ThreadPool::ThreadPool(std::size_t nThreads)
{
    if (nThreads == 0) nThreads = 1;
    m_workers.reserve(nThreads);
    for (std::size_t i = 0; i < nThreads; ++i) {
        m_workers.emplace_back([this] { WorkerLoop(); });
    }
}

ThreadPool::~ThreadPool()
{
    Stop();
}

std::future<void> ThreadPool::Push(std::function<void()> task)
{
    auto promise = std::make_shared<std::promise<void>>();
    std::future<void> fut = promise->get_future();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stop) {
            throw std::runtime_error("ThreadPool is stopped");
        }
        ++m_activeTasks;

        m_taskQueue.emplace([task = std::move(task), promise]() mutable {
            try {
                task();
                promise->set_value();
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        });
    }
    m_cv.notify_one();
    return fut;
}

void ThreadPool::WorkerLoop()
{
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return m_stop || !m_taskQueue.empty(); });

            if (m_stop && m_taskQueue.empty()) {
                return;               // завершаемся
            }
            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }
        task();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (--m_activeTasks == 0 && m_taskQueue.empty()) {
                m_doneCv.notify_all();
            }
        }
    }
}

void ThreadPool::WaitAll()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_doneCv.wait(lock, [this] {
        return m_taskQueue.empty() && m_activeTasks == 0;
    });
}

void ThreadPool::Stop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stop) return;
        m_stop = true;
    }
    m_cv.notify_all();

    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
}
