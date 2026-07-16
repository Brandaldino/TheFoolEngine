#pragma once

#include "CubeMap.h"
#include "Texture.h"

namespace TheFoolEngine
{
    class IBLUtils
    {
    public:
        static Ref<CubeMap> CreateIrradianceMap(Ref<CubeMap> skybox, uint32_t size = 32);
        static Ref<CubeMap> CreatePrefilteredMap(Ref<CubeMap> skybox, uint32_t size = 128);
        static Ref<Texture2D> CreateBRDFLUT(uint32_t size = 512);
    };
}