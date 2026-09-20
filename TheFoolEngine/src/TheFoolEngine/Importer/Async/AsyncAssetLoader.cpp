#include "tfpch.h"
#include "AsyncAssetLoader.h"

#include "../PBRModel.h"
#include "TheFoolEngine/Core/Job/JobSystem.h"
#include "../../Renderer/PBRRenderer.h"

#include <thread>

namespace TheFoolEngine
{
    AsyncAssetLoader& AsyncAssetLoader::Get()
    {
        static AsyncAssetLoader instance;
        return instance;
    }

    void AsyncAssetLoader::LoadModelAsync(const std::string& path, LoadCallback callback)
    {
        // Normalize: relative → absolute + unify separators + canonicalize
        std::filesystem::path fsPath(path);
        std::string key = std::filesystem::weakly_canonical(fsPath).string();

        // Cache hit: invoke the callback directly (sharing the same Model)
        auto it = m_Cache.find(key);
        if (it != m_Cache.end())
        {
            if (callback)
                callback(key, it->second);
            return;
        }

        JobSystem::Get().Submit([this, key, callback]()
            {
                TF_CORE_INFO("Async load on worker thread: {0}", std::hash<std::thread::id>{}(std::this_thread::get_id()));

                auto model = CreateRef<PBRModel>();
                std::filesystem::path fsPath(key);
                model->Import(fsPath);

                if (model->GetModelData().Meshes.empty())
                {
                    TF_ERROR("Async import failed: {0}", key);
                    return;
                }

                m_Completed.enqueue({ key, model, callback });
            });
    }

    void AsyncAssetLoader::ProcessCompleted()
    {
        AsyncLoadResult result;
        while (m_Completed.try_dequeue(result))
        {
            result.Model->UpLoad();
            PBRRenderer::DefaultTextureFill(result.Model);

            // Cache: if already exists (duplicate request), reuse cached one and discard newly loaded
            auto [it, inserted] = m_Cache.emplace(result.Path, result.Model);
            if (!inserted)
                result.Model = it->second;

            if (result.Callback)
                result.Callback(result.Path, result.Model);
        }
    }

}
