#include "tfpch.h"
#include "AsyncAssetLoader.h"

#include "TheFoolEngine/Importer/PBRModel.h"
#include "TheFoolEngine/Core/Job/JobSystem.h"
#include "TheFoolEngine/Renderer/PBRRenderer.h"
#include "TheFoolEngine/Importer/AssetRegistry.h"

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
        std::string key = AssetRegistry::NormalizePath(path);

        // Already loaded (in registry) → invoke callback directly (share ModelAsset)
        auto asset = AssetRegistry::Get().GetByPath(key);
        if (asset && asset->GetType() == AssetType::Model)
        {
            auto modelAsset = std::static_pointer_cast<ModelAsset>(asset);
            if (callback)
                callback(key, modelAsset->GetModel());
            return;
        }

        // Not loaded → background import
        JobSystem::Get().Submit([this, path, key, callback]()
            {
                auto model = CreateRef<PBRModel>();
                model->Import(std::filesystem::path(path));

                if (model->GetModelData().Meshes.empty())
                {
                    TF_CORE_ERROR("Async import failed: {0}", path);
                    return;
                }
                m_Completed.enqueue({ key,model, callback });
            });
    }

    void AsyncAssetLoader::ProcessCompleted()
    {
        AsyncLoadResult result;
        while (m_Completed.try_dequeue(result))
        {
            result.Model->UpLoad();
            PBRRenderer::DefaultTextureFill(result.Model);

            // Register in AssetRegistry (deduplicate by path)
            UUID id = AssetRegistry::HashPath(result.Path);
            auto modelAsset = CreateRef<ModelAsset>(result.Model, id, result.Path);
            UUID registeredID = AssetRegistry::Get().Register(modelAsset, result.Path);

            // Concurrent duplicate request: already exists → use the one in registry (discard the newly loaded one)
            Ref<PBRModel> model = result.Model;
            if (registeredID != id)
            {
                auto existing = AssetRegistry::Get().GetAsset(registeredID);
                model = std::static_pointer_cast<ModelAsset>(existing)->GetModel();
            }

            if (result.Callback)
                result.Callback(result.Path, model);
        }
    }

}
