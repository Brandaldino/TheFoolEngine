#pragma once
#include "Texture.h"

#include <functional>
#include <type_traits>

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
        bool IsTransient = false;

        bool operator==(const TextureDesc& other) const
        {
            return  this->Width == other.Width &&
                this->Height == other.Height &&
                this->Format == other.Format &&
                this->LayerCount == other.LayerCount &&
                this->MipLevels == other.MipLevels &&
                this->Samples == other.Samples &&
                this->Type == other.Type &&
                this->IsTransient == other.IsTransient;
        }

        uint64_t GetHash() const
        {
            std::size_t seed = 0;
            auto hash_combine = [&](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                seed ^= std::hash<T>{}(val)+0x9e3779b9 + (seed << 6) + (seed >> 2);
                };

            hash_combine(Width);
            hash_combine(Height);
            hash_combine(Format);
            hash_combine(LayerCount);
            hash_combine(MipLevels);
            hash_combine(Samples);
            hash_combine(Type);
            hash_combine(IsTransient);

            return static_cast<uint64_t>(seed);
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