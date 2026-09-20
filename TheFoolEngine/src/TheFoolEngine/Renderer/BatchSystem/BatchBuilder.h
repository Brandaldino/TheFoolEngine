#pragma once

#include "../../Importer/PBRModel.h"
#include "../../Core/Utils/HashUtils.h"
#include "../Pass/Pass.h"

#include <glm/glm.hpp>

namespace TheFoolEngine
{
    class VertexArray;
    class Texture2D;

    struct BatchKey
    {
        // vertex
        Ref<VertexArray> VAO;
        uint32_t IndexCount = 0;

        // texture
        Ref<Texture2D> Albedo;
        Ref<Texture2D> Normal;
        Ref<Texture2D> MetallicRoughness;
        Ref<Texture2D> AO;

        // material factor
        glm::vec4 AlbedoFactor;
        glm::vec2 MetallicRoughnessFactor;  // metallic + roughness
        float AOStrength;

        bool operator==(const BatchKey& other) const
        {
            return this->VAO == other.VAO &&
                this->IndexCount == other.IndexCount &&
                this->Albedo == other.Albedo &&
                this->Normal == other.Normal &&
                this->MetallicRoughness == other.MetallicRoughness &&
                this->AO == other.AO &&
                this->AlbedoFactor == other.AlbedoFactor &&
                this->MetallicRoughnessFactor == other.MetallicRoughnessFactor &&
                this->AOStrength == other.AOStrength;
        }

        uint64_t GetHash() const
        {
            return HashUtils::HashFields(
                (uintptr_t)VAO.get(),
                IndexCount,
                (uintptr_t)Albedo.get(),
                (uintptr_t)Normal.get(),
                (uintptr_t)MetallicRoughness.get(),
                (uintptr_t)AO.get(),
                AlbedoFactor.x, AlbedoFactor.y, AlbedoFactor.z, AlbedoFactor.w,
                MetallicRoughnessFactor.x, MetallicRoughnessFactor.y,
                AOStrength);
        }
    };

    struct BatchKeyHash
    {
        std::size_t operator()(const BatchKey& key) const { return (std::size_t)key.GetHash(); };
    };

    struct Batch
    {
        BatchKey Key;
        std::vector<glm::mat4> Matrices;    // Instance matrix
        uint32_t RenderOffset = 0;  // SSBO offset (filled during the fill phase)
    };

    class BatchBuilder
    {
    public:
        void Reset() { m_Batches.clear(); };

        void AddRenderable(const PBRRenderProxy& proxy);
        void Sort();

        std::vector<Batch>& GetBatches() { return m_Sorted; };
        std::size_t GetBatchCount() const { return m_Sorted.size(); };
    private:
        std::unordered_map<BatchKey, Batch, BatchKeyHash> m_Batches;
        std::vector<Batch> m_Sorted;
    };
}