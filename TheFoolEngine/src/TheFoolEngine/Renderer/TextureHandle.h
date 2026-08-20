#pragma once

#include "RenderGraphTypes.h"

namespace TheFoolEngine
{
    class TextureHandle
    {
    public:
        bool IsValid() const { return PoolIndex != UINT32_MAX; };

        bool operator==(const TextureHandle& other) const
        {
            return this->PoolIndex == other.PoolIndex;
        }

        uint64_t GetHash() const
        {
            return PoolIndex;
        }

    public:
        TextureDesc Desc;
        uint32_t PoolIndex = UINT32_MAX;
        std::string Name; // for Debug
    };

    struct TextureHandleHash
    {
        std::size_t operator()(const TextureHandle& handle) const
        {
            return (std::size_t)handle.GetHash();
        }
    };
}