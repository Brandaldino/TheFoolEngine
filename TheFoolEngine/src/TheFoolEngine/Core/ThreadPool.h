#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <random>
#include <vector>
#include <memory>

#include "concurrentqueue.h"

namespace TheFoolEngine
{
    class ThreadPool
    {
        using Task = std::function<void()>;	// work type
        struct alignas(std::hardware_destructive_interference_size)Worker
        {
            moodycamel::ConcurrentQueue<Task> queue;	// self work queue
            std::thread thread;	// work thread
            ThreadPool* pool = nullptr;
            std::size_t id = 0;	// work id
        };
    public:
        explicit ThreadPool(std::size_t = std::thread::hardware_concurrency() - 2);
        ~ThreadPool();

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        template <typename F, typename ... Args>
        auto Submit(F&& f, Args&&... args)
            -> std::future<typename std::invoke_result_t<F, Args...>>;

        void WaitAll();
        std::size_t GetThreadCount() const { return m_Workers.size(); };
    private:
        void WorkerLoop(Worker* worker);
        std::size_t ChooseWorkerForSubmission();
    private:
        std::vector<std::unique_ptr<Worker>> m_Workers;
        std::atomic<bool> m_Stop{ false };
        std::condition_variable m_Condition; // Global synchronization
        std::mutex m_GlobalMtx;	// protect global state
        std::atomic<std::size_t> m_ActiveTasks{ 0 };
    };

    template <typename F, typename ... Args>
    auto ThreadPool::Submit(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using ReturnType = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<ReturnType> res = task->get_future();

        Task wrapped_task = [task]() { (*task)(); };
        std::size_t worker_id = ChooseWorkerForSubmission();

        auto& worker = *m_Workers[worker_id];
        {
            std::lock_guard<std::mutex> lock(m_GlobalMtx);
            worker.queue.enqueue(std::move(wrapped_task));
            m_ActiveTasks.fetch_add(1, std::memory_order_release);
            m_Condition.notify_one();
        }

        return res;
    }
}
