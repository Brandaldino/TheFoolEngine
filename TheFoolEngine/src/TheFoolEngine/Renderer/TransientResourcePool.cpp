#include "tfpch.h"
#include "TransientResourcePool.h"

#include "FrameBuffer.h"
#include "PointShadowMap.h"

namespace TheFoolEngine
{

    uint32_t TransientResourcePool::Allocate(const TextureDesc& desc)
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
        if (desc.Type == RenderTargetType::DepthArray)
        {
            FrameBufferSpecification spec;
            spec.Width = desc.Width;
            spec.Height = desc.Height;
            spec.DepthOnly = true;
            spec.LayerCount = desc.LayerCount;
            entry.FrameBuffer = FrameBuffer::Create(spec);
        }
        else if (desc.Type == RenderTargetType::CubeMapArray)
            entry.PointShadowMap = PointShadowMap::Create(desc.Width, desc.LayerCount);

        entry.InUse = true;
        m_Pool.emplace_back(std::move(entry));
        return (uint32_t)m_Pool.size() - 1;
    }

    void TransientResourcePool::ResetFrame()
    {
        for (uint32_t i = 0; i < m_Pool.size(); ++i)
        {
            m_FreeList[m_Pool[i].Desc] = i;
            m_Pool[i].InUse = false;
        }
    }

    void TransientResourcePool::Release(uint32_t poolIndex)
    {
        if (poolIndex >= m_Pool.size())
            return;

        m_FreeList[m_Pool[poolIndex].Desc] = poolIndex;
        m_Pool[poolIndex].InUse = false;
    }

    Ref<FrameBuffer> TransientResourcePool::GetFrameBuffer(uint32_t poolIndex) const
    {
        if (poolIndex >= m_Pool.size())
            return nullptr;

        return m_Pool[poolIndex].FrameBuffer;
    }

    Ref<PointShadowMap> TransientResourcePool::GetPointShadowMap(uint32_t poolIndex) const
    {
        if (poolIndex >= m_Pool.size())
            return nullptr;

        return m_Pool[poolIndex].PointShadowMap;
    }

}