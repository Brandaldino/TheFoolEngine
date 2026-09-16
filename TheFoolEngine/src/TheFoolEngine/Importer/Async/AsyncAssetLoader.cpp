#include "tfpch.h"
#include "AsyncAssetLoader.h"

#include "../PBRModel.h"
#include "TheFoolEngine/Core/Job/JobSystem.h"
#include "../../Renderer/PBRRenderer.h"

#include <thread>

namespace TheFoolEngine
{

    void AsyncAssetLoader::LoadModelAsync(const std::string& path, LoadCallback callback)
    {
        JobSystem::Get().Submit([this, path, callback]()
            {
                TF_CORE_INFO("Async load on worker thread: {0}", std::hash<std::thread::id>{}(std::this_thread::get_id()));

                auto model = CreateRef<PBRModel>();
                std::filesystem::path fsPath(path);
                model->Import(fsPath);

                if (model->GetModelData().Meshes.empty())
                {
                    TF_ERROR("Async import failed: {0}", path);
                    return;
                }

                m_Completed.enqueue({ path, model, callback });
            });
    }

    void AsyncAssetLoader::ProcessCompleted()
    {
        AsyncLoadResult result;
        while (m_Completed.try_dequeue(result))
        {
            result.Model->UpLoad();
            PBRRenderer::DefaultTextureFill(result.Model);
            if (result.Callback)
                result.Callback(result.Path, result.Model);
        }
    }

}
