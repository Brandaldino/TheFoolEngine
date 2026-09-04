#pragma once

#include "../Core/Base.h"

#include "RenderGraphTypes.h"

namespace TheFoolEngine
{
    class FrameBuffer;
    class PointShadowMap;

    class TransientResourcePool
    {
    public:
        uint32_t Allocate(const TextureDesc& desc);
        void ResetFrame(); // Recycled at frame end
        void Release(uint32_t poolIndex);

        Ref<FrameBuffer> GetFrameBuffer(uint32_t poolIndex) const;
        Ref<PointShadowMap> GetPointShadowMap(uint32_t poolIndex) const;
    private:
        struct PoolEntry
        {
            TextureDesc Desc;
            Ref<FrameBuffer> FrameBuffer; // DepthArray 
            Ref<PointShadowMap> PointShadowMap; // CubeArray
            bool InUse = false;
        };

        std::vector<PoolEntry> m_Pool;
        std::unordered_map<TextureDesc, uint32_t, TextureDescHash> m_FreeList;
    };
}