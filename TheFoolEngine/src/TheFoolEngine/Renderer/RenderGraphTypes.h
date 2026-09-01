#pragma once
#include "Texture.h"

namespace TheFoolEngine
{
    enum class RenderTargetType : uint8_t
    {
        Color,
        DepthArray,
        CubeMapArray
    };

    struct TextureDesc
    {
        uint32_t Width = 0, Height = 0;
        TextureFormat Format = RGBA16F;
        uint32_t LayerCount = 1;
        uint32_t MipLevels = 1;
        uint32_t Samples = 1;
        RenderTargetType Type = RenderTargetType::Color;
        bool IsDepth = false;   // depth attachment
        bool IsCubemap = false; // skybox
        bool IsArray = false; // shadow array

        bool operator==(const TextureDesc& other) const
        {
            return  this->Width == other.Width &&
                this->Height == other.Height &&
                this->Format == other.Format &&
                this->LayerCount == other.LayerCount &&
                this->MipLevels == other.MipLevels &&
                this->Samples == other.Samples &&
                this->IsDepth == other.IsDepth &&
                this->IsCubemap == other.IsCubemap &&
                this->IsArray == other.IsArray;
        }

        uint64_t GetHash() const
        {
            uint64_t h = 0;
            h ^= (uint64_t)Width;
            h ^= (uint64_t)Height << 16;
            h ^= (uint64_t)Format << 32;
            h ^= (uint64_t)LayerCount << 40;
            h ^= (uint64_t)MipLevels << 48;
            h ^= (uint64_t)Samples << 56;
            h ^= IsDepth ? 1ULL << 5 : 0;
            h ^= IsCubemap ? 1ULL << 6 : 0;
            h ^= IsArray ? 1ULL << 7 : 0;
            return h;
        }
    };
    // for std::unordered_map
    struct TextureDescHash
    {
        std::size_t operator()(const TextureDesc& desc) const
        {
            return (std::size_t)desc.GetHash();
        }
    };

}