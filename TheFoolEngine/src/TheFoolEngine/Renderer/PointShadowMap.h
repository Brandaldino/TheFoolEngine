#pragma once

#include "TheFoolEngine/Core/Base.h"

namespace TheFoolEngine
{
    class PointShadowMap
    {
    public:
        virtual ~PointShadowMap() = default;

        virtual void BindFace(uint32_t layer, uint32_t face) = 0;
        virtual void Bind(uint32_t slot = 0) = 0;
        virtual void Unbind() = 0;
        virtual uint32_t GetRendererID() const = 0;

        static Ref<PointShadowMap> Create(uint32_t size, uint32_t layerCount);
    };
}