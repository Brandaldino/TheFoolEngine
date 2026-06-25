#include "tfpch.h"
#include "PBRMaterialManager.h"

namespace TheFoolEngine
{
    PBRMaterialManager* PBRMaterialManager::Get()
    {
        static PBRMaterialManager instance;
        return &instance;
    }

    void PBRMaterialManager::RegisterMaterial(uint32_t hashkey, Ref<PBRMaterial> material)
    {
        auto pos = m_MaterialCache.find(hashkey);
        if (pos == m_MaterialCache.end())
            m_MaterialCache[hashkey] = material;
    }

    Ref<PBRMaterial> PBRMaterialManager::GetMaterial(uint32_t hashkey) const
    {
        auto pos = m_MaterialCache.find(hashkey);
        if (pos == m_MaterialCache.end())
            return nullptr;
        return pos->second;
    }

}