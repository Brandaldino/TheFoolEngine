#include "tfpch.h"
#include "ThreadPool.h"

#include <emmintrin.h>


namespace TheFoolEngine
{
    ThreadPool::ThreadPool(std::size_t numThreads)
    {
        numThreads = std::clamp(
            numThreads,
            static_cast<std::size_t>(1u),
            static_cast<std::size_t>(std::thread::hardware_concurrency() - 2));
        m_Workers.reserve(numThreads);
        for (std::size_t i = 0; i < numThreads; ++i)
        {
            auto worker = std::make_unique<Worker>();
            worker->pool = this;
            worker->id = i;
            worker->thread = std::thread(&ThreadPool::WorkerLoop, this, worker.get());
            m_Workers.push_back(std::move(worker));
        }
    }

    ThreadPool::~ThreadPool()
    {
        WaitAll();

        {
            std::lock_guard<std::mutex> lock(m_GlobalMtx);
            m_Stop = true;
        }
        m_Condition.notify_all();

        for (auto& worker_ptr : m_Workers)
        {
            if (worker_ptr->thread.joinable())
                worker_ptr->thread.join();
        }
    }

    void ThreadPool::WaitAll()
    {
        std::unique_lock<std::mutex> lock(m_GlobalMtx);
        m_Condition.wait(lock, [this] {
            for (auto& w : m_Workers)
                if (w->queue.size_approx() > 0)
                    return false;
            return m_ActiveTasks.load(std::memory_order_acquire) == 0;
            });
    }

    std::size_t ThreadPool::ChooseWorkerForSubmission()
    {
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<std::size_t> dist(0, m_Workers.size() - 1);
        return dist(rng);
    }

    void ThreadPool::WorkerLoop(Worker* worker)
    {
        constexpr std::size_t SPIN_COUNT = 16;

        while (true)
        {
            Task task;
            bool got_task = false;

            if (worker->queue.try_dequeue(task))
                got_task = true;

            if (!got_task)
            {
                for (std::size_t i = 0; i < m_Workers.size();++i)
                {
                    std::size_t victim_id = (worker->id + i) % m_Workers.size();
                    if (victim_id == worker->id)
                        continue;

                    auto& victim = *m_Workers[victim_id];
                    if (victim.queue.try_dequeue(task))
                    {
                        got_task = true;
                        break;
                    }
                }
            }

            if (!got_task)
            {
                if (m_Stop.load(std::memory_order_acquire))
                    return;

                bool any_task = false;

                for (auto& w : m_Workers)
                {
                    if (w->queue.size_approx() > 0)
                    {
                        any_task = true;
                        break;
                    }
                }

                if (any_task)
                    continue;

                for (std::size_t i = 0;i < SPIN_COUNT;++i)
                {
                    for (auto& w : m_Workers)
                    {
                        if (w->queue.size_approx() > 0)
                        {
                            any_task = true;
                            break;
                        }
                    }

                    if (any_task)
                        break;

                    _mm_pause();
                }

                if (any_task)
                    continue;

                if (!got_task)
                {
                    std::unique_lock<std::mutex> lock(m_GlobalMtx);
                    m_Condition.wait(lock, [&] {
                        if (m_Stop.load(std::memory_order_acquire))
                            return true;
                        for (auto& w : m_Workers)
                            if (w->queue.size_approx() > 0)
                                return true;
                        return false;
                        });
                    continue;
                }
            }

            if (got_task)
            {
                task();
                if (m_ActiveTasks.fetch_sub(1, std::memory_order_acq_rel) == 1)
                    m_Condition.notify_all();
                else
                    m_Condition.notify_one();
            }
        }
    }
}
