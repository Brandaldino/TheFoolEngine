#pragma once

#include "TheFoolEngine/Core/Base.h"
#include "concurrentqueue.h"

namespace TheFoolEngine
{
    class PBRModel;

    using LoadCallback = std::function<void(const std::string& path, Ref<PBRModel> model)>;

    struct AsyncLoadResult
    {
        std::string Path;
        Ref<PBRModel> Model;
        LoadCallback Callback;
    };

    class AsyncAssetLoader
    {
    public:
        static AsyncAssetLoader& Get();

        void LoadModelAsync(const std::string& path, LoadCallback callback);
        void ProcessCompleted();
    private:
        moodycamel::ConcurrentQueue<AsyncLoadResult> m_Completed;
    };
}