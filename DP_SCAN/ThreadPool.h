#ifndef ThreadPool_H
#define ThreadPool_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <memory>

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t nThreads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Кладёт задачу в очередь. Возвращает future для ожидания результата.
    std::future<void> Push(std::function<void()> task);

    // Дождаться выполнения всех задач, поставленных к этому моменту
    void WaitAll();

    void Stop();

private:
    void WorkerLoop();

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_taskQueue;

    std::mutex m_mutex;
    std::condition_variable m_cv;          // для workers: есть задача / стоп
    std::condition_variable m_doneCv;      // для WaitAll

    bool m_stop{false};
    std::atomic<std::size_t> m_activeTasks{0};
};

#endif
