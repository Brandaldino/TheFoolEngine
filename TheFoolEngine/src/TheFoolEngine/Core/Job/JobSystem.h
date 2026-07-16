#pragma once

#include "../ThreadPool.h"

namespace TheFoolEngine
{
    class JobSystem
    {
    public:
        struct JobHandleImpl : public std::enable_shared_from_this<JobHandleImpl>
        {
            std::atomic<int> pendingDeps{ 0 };
            std::atomic<bool> done{ false };
            std::function<void()> task;
            std::vector<std::shared_ptr<JobHandleImpl>> continuations;
            std::mutex contMutex;

            void AddContinuation(std::shared_ptr<JobHandleImpl> cont);
            void OnDependencyFinished();
            void Finalize();
            void Schedule();
        };

        class JobHandle
        {
        public:
            JobHandle() = default;
            explicit JobHandle(std::shared_ptr<JobHandleImpl> impl);
            ~JobHandle() = default;

            JobHandle(const JobHandle&) = default;
            JobHandle& operator=(const JobHandle&) = default;
            JobHandle(JobHandle&&) = default;
            JobHandle& operator=(JobHandle&&) = default;

            // Wait for the task and its dependencies to be completed.
            void Wait() const;

            // Check if completed
            bool IsDone() const;

            std::shared_ptr<JobHandleImpl> GetImpl() const { return m_Impl; };

        private:
            std::shared_ptr<JobHandleImpl> m_Impl;
        };

        // Init / shutdown 
        static void Initialize(std::size_t numThreads = std::thread::hardware_concurrency() - 2);
        static void Shutdown();
        static JobSystem& Get();

        template<typename F>
        JobHandle Submit(F&& func);

        // submit independent task
        template<typename F>
        JobHandle SubmitWithDeps(F&& func, std::vector<JobHandle> deps);

        // Parallel processing of arrays (automatic data partitioning)
        template<typename T, typename Func>
        JobHandle ParallelFor(std::vector<T>& data, Func&& func, std::size_t chunkSize = 0);

        // Wait for the completion of all submitted top-level tasks
        void WaitAll();

        // Directly access the underlying thread pool (for advanced operations)
        ThreadPool& GetThreadPool() { return m_ThreadPool; };

        ~JobSystem() = default;
    private:
        JobSystem(std::size_t numThreads);
        JobSystem(const JobSystem&) = delete;
        JobSystem& operator=(const JobSystem&) = delete;

        ThreadPool m_ThreadPool;
        std::mutex m_PendingMutex;
        std::vector<JobHandle> m_PendingHandles;
    };
    //////////////////////////////////////////////////////////////////
    template<typename F>
    JobSystem::JobHandle JobSystem::Submit(F&& func)
    {
        return SubmitWithDeps(std::forward<F>(func), {});
    }

    template<typename F>
    JobSystem::JobHandle JobSystem::SubmitWithDeps(F&& func, std::vector<JobHandle> deps)
    {
        auto impl = std::make_shared<JobHandleImpl>();
        impl->task = std::forward<F>(func);
        // Prevent dependency dependency from being completed during the addition process: pendingDeps = dependency count + 1, and finally release it through Finalize.
        impl->pendingDeps.store(static_cast<int>(deps.size()) + 1, std::memory_order_release);

        for (auto& dep : deps)
        {
            if (dep.GetImpl())
                dep.GetImpl()->AddContinuation(impl);
        }

        impl->Finalize();

        JobHandle handle(impl);
        {
            std::lock_guard<std::mutex> lock(m_PendingMutex);
            m_PendingHandles.push_back(handle);
        }

        return handle;
    }

    template<typename T, typename Func>
    JobSystem::JobHandle JobSystem::ParallelFor(std::vector<T>& data, Func&& func, std::size_t chunkSize)
    {
        const std::size_t n = data.size();
        if (n == 0)
            return JobHandle();

        if (chunkSize == 0)
        {
            const std::size_t numThreads = m_ThreadPool.GetThreadCount();
            chunkSize = std::max<std::size_t>(1, (n + numThreads - 1) / numThreads);
        }

        std::vector<JobHandle> subJobs;
        for (std::size_t i = 0;i < n; i += chunkSize)
        {
            std::size_t start = i;
            std::size_t end = std::min(i + chunkSize, n);
            subJobs.push_back(Submit([start, end, &data, &func]
                {
                    for (std::size_t j = start; j < end; ++j)
                        func(data[j]);
                }));
        }

        // Return an empty task that depends on all sub-tasks, representing the overall completion.
        return SubmitWithDeps([] {}, subJobs);
    }
}