#pragma once

#include "stb_image.h"
#include "concurrentqueue.h"

namespace TheFoolEngine
{
    class IOThread
    {
    public:
        using CallBack = std::function<void(std::vector<stbi_uc> data)>;

        void Start();
        void Stop();

        void Enqueue(std::filesystem::path path, CallBack callback);
    private:
        void Worker();
    private:
        std::thread m_Thread;
        moodycamel::ConcurrentQueue<std::pair<std::filesystem::path, CallBack>> m_Queue;
        std::atomic<bool> m_Running{ false };
    };
}

