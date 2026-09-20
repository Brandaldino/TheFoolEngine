#include "tfpch.h"
#include "BatchBuilder.h"

#include "../../Renderer/PBRRenderer.h"

namespace TheFoolEngine
{
    void BatchBuilder::AddRenderable(const PBRRenderProxy& proxy)
    {
        auto& modelData = proxy.Model->GetModelData();
        auto& vas = proxy.Model->GetVertexArray();
        auto& meshes = modelData.Meshes;
        auto& texSets = modelData.Textures;

        const auto& prts = PBRRenderer::GetDefaultTexture();

        for (std::size_t i = 0;i < vas.size(); ++i)
        {
            auto& texSet = texSets[meshes[i].MaterialIndex];
            BatchKey key;
            key.VAO = vas[i];
            key.IndexCount = vas[i]->GetIndexBuffer()->GetCount();;
            key.Albedo = texSet.AlbedoMap ? texSet.AlbedoMap : prts.AlbedoMap;
            key.Normal = texSet.NormalMap ? texSet.NormalMap : prts.NormalMap;
            key.MetallicRoughness = texSet.MetallicRoughnessMap ? texSet.MetallicRoughnessMap : prts.MetallicRoughnessMap;
            key.AO = texSet.AOMap ? texSet.AOMap : prts.AOMap;
            key.AlbedoFactor = glm::vec4(texSet.AlbedoFactor, 1.0f);
            key.MetallicRoughnessFactor = { texSet.MetallicFactor, texSet.RoughnessFactor };
            key.AOStrength = texSet.AOStrength;

            auto& batch = m_Batches[key];
            batch.Key = key;
            batch.Matrices.push_back({ proxy.Transform * meshes[i].NodeTransform });
        }
    }

    // State locality sorting: shader → material → texture → VAO
    // Currently single shader; sort key = (texture → material factors → VAO)
    void BatchBuilder::Sort()
    {
        m_Sorted.clear();
        m_Sorted.reserve(m_Batches.size());
        for (auto& [key, batch] : m_Batches)
            m_Sorted.push_back(std::move(batch));
        // Sort by the 4 texture pointers (state-switch locality)
        std::sort(m_Sorted.begin(), m_Sorted.end(), [](const Batch& a, const Batch& b)
            {
                if (a.Key.Albedo != b.Key.Albedo)
                    return a.Key.Albedo.get() < b.Key.Albedo.get();
                if (a.Key.Normal != b.Key.Normal)
                    return a.Key.Normal.get() < b.Key.Normal.get();
                if (a.Key.MetallicRoughness != b.Key.MetallicRoughness)
                    return a.Key.MetallicRoughness.get() < b.Key.MetallicRoughness.get();
                if (a.Key.AO != b.Key.AO)
                    return a.Key.AO.get() < b.Key.AO.get();
                return a.Key.VAO.get() < b.Key.VAO.get();
            });
    }

}