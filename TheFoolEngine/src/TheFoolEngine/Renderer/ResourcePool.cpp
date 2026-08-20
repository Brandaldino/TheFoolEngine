#include "tfpch.h"
#include "ResourcePool.h"

#include "Texture.h"

namespace TheFoolEngine
{

    uint32_t ResourcePool::Allocate(const TextureDesc& desc)
    {
        auto it = m_FreeList.find(desc);
        if (it != m_FreeList.end())
        {
            uint32_t index = it->second;
            m_FreeList.erase(it);
            m_Pool[index].InUse = true;
            return index;
        }

        PoolEntry entry;
        entry.Desc = desc;
        entry.Texture = Texture2D::Create(desc.Width, desc.Height, desc.Format);
        entry.InUse = true;
        m_Pool.emplace_back(std::move(entry));
        return (uint32_t)m_Pool.size() - 1;
    }

    void ResourcePool::Release(uint32_t poolIndex)
    {
        if (poolIndex >= m_Pool.size())
            return;

        m_FreeList[m_Pool[poolIndex].Desc] = poolIndex;
        m_Pool[poolIndex].InUse = false;
    }

    void ResourcePool::ResetFrame()
    {
        for (auto& entry : m_Pool)
            entry.InUse = false;
    }

    uint32_t ResourcePool::Register(const TextureDesc& desc, Ref<Texture2D> texture)
    {
        PoolEntry entry;
        entry.Desc = desc;
        entry.Texture = texture;
        entry.InUse = true;
        m_Pool.emplace_back(std::move(entry));
        return (uint32_t)m_Pool.size() - 1;
    }

    uint32_t ResourcePool::GetRendererID(uint32_t poolIndex) const
    {
        if (poolIndex >= m_Pool.size())
            return NULL;

        return m_Pool[poolIndex].Texture->GetRendererID();
    }

    Ref<Texture2D> ResourcePool::GetTexture(uint32_t poolIndex) const
    {
        if (poolIndex >= m_Pool.size())
            return nullptr;

        return m_Pool[poolIndex].Texture;
    }

}
