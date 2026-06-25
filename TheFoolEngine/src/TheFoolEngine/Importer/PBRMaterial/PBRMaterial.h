#pragma once

#include "../MaterialData/PBRMaterialData.h"

namespace TheFoolEngine
{
    class PBRMaterial
    {
    public:
        PBRMaterial() = default;
        ~PBRMaterial() = default;

        void SetHash(uint32_t hash) { m_HashKey = hash; };
        uint32_t GetHash() const { return m_HashKey; }

        void SetMaterialData(const PBRMaterialTextureSet& data) { m_MaterialTextureSet = data; };
        const PBRMaterialTextureSet& GetMaterialData() const { return m_MaterialTextureSet; };
    private:
        uint32_t m_HashKey;
        PBRMaterialTextureSet m_MaterialTextureSet;
    };

    class PBRMaterialBuilder
    {
    public:
        void Build(Ref<PBRMaterial> material);
    private:
        uint32_t GenerateHash(const PBRMaterialTextureSet& data);
    };
}