#pragma once

#include "PBRMaterial.h"

namespace TheFoolEngine
{
    class PBRMaterialManager
    {
    public:
        static PBRMaterialManager* Get();

        void RegisterMaterial(uint32_t hashkey, Ref<PBRMaterial> material);
        // Get Material by hash key
        Ref<PBRMaterial> GetMaterial(uint32_t hashkey) const;
    private:
        std::unordered_map<uint32_t, Ref<PBRMaterial>> m_MaterialCache;
    };
}