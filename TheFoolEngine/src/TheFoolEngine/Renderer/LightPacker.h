#pragma once
#include "Light.h"
#include "ShadowTypes.h"

namespace TheFoolEngine
{

    namespace LightPacker
    {

        GPULight PackDirection(const DirectionLight& light, int shadowIndex = -1);
        GPULight PackSpot(const SpotLight& light, int shadowIndex = -1);
        GPULight PackPoint(const PointLight& light, int shadowIndex = -1);
        void SetGPULightFBO(uint32_t ubo, std::vector<GPULight> lights);

    }

}