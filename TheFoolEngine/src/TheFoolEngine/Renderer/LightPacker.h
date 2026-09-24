#pragma once
#include "Light.h"
#include "ShadowTypes.h"

namespace TheFoolEngine
{
    class StorageBuffer;

    namespace LightPacker
    {
        GPULight PackDirection(const DirectionLight& light, int shadowIndex = -1);
        GPULight PackSpot(const SpotLight& light, int shadowIndex = -1);
        GPULight PackPoint(const PointLight& light, int shadowIndex = -1);
        void SetGPULightFBO(Ref<StorageBuffer> ubo, std::vector<GPULight> lights);
    }

}