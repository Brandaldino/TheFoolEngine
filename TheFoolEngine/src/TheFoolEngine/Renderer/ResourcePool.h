#pragma once

#include "RenderGraphTypes.h"

#include <vector>
#include <unordered_map>

namespace TheFoolEngine
{

    class ResourcePool
    {
    public:
        // Create or reuse: reuse an existing texture if the same desc already exists
        uint32_t Allocate(const TextureDesc& desc);     // return poolindex
        void Release(uint32_t poolIndex);   // Marking is reusable

        void ResetFrame();
        uint32_t Register(const TextureDesc& desc, Ref<Texture2D> texture);

        // Inspect the actual texture in the pool
        uint32_t GetRendererID(uint32_t poolIndex) const;
        Ref<Texture2D> GetTexture(uint32_t poolIndex) const;
    private:
        struct PoolEntry
        {
            TextureDesc Desc;
            Ref<Texture2D> Texture;
            bool InUse = false; // is occupied in the current frame 
        };
        std::vector<PoolEntry> m_Pool;
        std::unordered_map<TextureDesc, uint32_t, TextureDescHash> m_FreeList;
    };
    
}