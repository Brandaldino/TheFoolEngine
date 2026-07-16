#include "tfpch.h"
#include "JobSystem.h"
#include <cassert>

namespace TheFoolEngine
{
    // ================================ JobHandleImpl ================================
    void JobSystem::JobHandleImpl::AddContinuation(std::shared_ptr<JobHandleImpl> cont)
    {
        bool alreadyDone = done.load(std::memory_order_acquire);
        if (!alreadyDone)
        {
            std::lock_guard<std::mutex> lock(contMutex);
            // After locking, check again to prevent the situation where it is completed just before locking.
            if (done.load(std::memory_order_acquire))
                alreadyDone = true;
            else
                continuations.push_back(std::move(cont));
        }
        if (alreadyDone)
            cont->OnDependencyFinished();
    }

    void JobSystem::JobHandleImpl::OnDependencyFinished()
    {
        if (pendingDeps.fetch_sub(1, std::memory_order_acq_rel) == 1)
            Schedule();
    }

    void JobSystem::JobHandleImpl::Finalize()
    {
        // The value of pendingDeps was initially set to deps.size() + 1, and now that extra count has been subtracted.
        if (pendingDeps.fetch_sub(1, std::memory_order_acq_rel) == 1)
            Schedule();	// All the initial dependencies have been completed, and the Finalize has been called. It is now possible to schedule.
    }

    void JobSystem::JobHandleImpl::Schedule()
    {
        auto this_shared = shared_from_this();
        JobSystem::Get().GetThreadPool().Submit([this_shared]
            {
                if (this_shared->task)
                    this_shared->task();

                this_shared->done.store(true, std::memory_order_release);

                std::lock_guard<std::mutex> lock(this_shared->contMutex);
                for (auto& cont : this_shared->continuations)
                    cont->OnDependencyFinished();
            }
        );
    }

    // ================================ JobHandle ================================
    JobSystem::JobHandle::JobHandle(std::shared_ptr<JobHandleImpl> impl)
        : m_Impl(std::move(impl)) {
    }

    void JobSystem::JobHandle::Wait() const
    {
        if (m_Impl)
        {
            // TODD: Simple spin wait. In production environment, thread assistance can be added.
            while (!m_Impl->done.load(std::memory_order_acquire))
                std::this_thread::yield();
        }
    }

    bool JobSystem::JobHandle::IsDone() const
    {
        return m_Impl && m_Impl->done.load(std::memory_order_acquire);
    }

    // ================================ JobSystem ================================
    static std::unique_ptr<JobSystem> s_Instance;

    void JobSystem::Initialize(std::size_t numThreads)
    {
        assert(!s_Instance);
        s_Instance.reset(new JobSystem(numThreads));
    }

    void JobSystem::Shutdown()
    {
        if (s_Instance)
        {
            s_Instance->m_ThreadPool.WaitAll();
            s_Instance.reset();
        }
    }

    JobSystem& JobSystem::Get()
    {
        assert(s_Instance);
        return *s_Instance;
    }

    JobSystem::JobSystem(std::size_t numThreads)
        : m_ThreadPool(numThreads) {
    }

    void JobSystem::WaitAll()
    {
        std::vector<JobHandle> handles;
        {
            std::lock_guard<std::mutex> lock(m_PendingMutex);
            handles = m_PendingHandles;
        }

        for (auto& h : handles)
            h.Wait();

        {
            std::lock_guard<std::mutex> lock(m_PendingMutex);
            m_PendingHandles.clear();
        }
    }

}