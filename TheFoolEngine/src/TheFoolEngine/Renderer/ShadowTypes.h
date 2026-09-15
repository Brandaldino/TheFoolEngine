#pragma once

#include "Light.h"
#include "Shader.h"

namespace TheFoolEngine
{
    constexpr uint32_t MAX_SHADOW_LIGHTS = 8;
    constexpr uint32_t MAX_DIR_SPOT_SHADOWS = 4;
    constexpr uint32_t MAX_POINT_SHADOWS = 2;
    constexpr uint32_t SHADOWMAP_SIZE = 1024;
    constexpr uint32_t DIR_SPOT_SHADOW_SIZE = 1024;
    constexpr uint32_t POINT_SHADOW_SIZE = 512;
    constexpr uint32_t NR_LIGHTS = 10;

    struct PointLightShadowData
    {
        glm::vec3 LightPosition;
        glm::mat4 ShadowViews[6];
        glm::mat4 ShadowProj;
        float FarPlane;
    };

    struct PointShadowData
    {
        PointLightShadowData Lights[MAX_SHADOW_LIGHTS];
        uint32_t Count = 0;
    };

    struct LightGPUBlock
    {
        GPULight Lights[NR_LIGHTS];
        int32_t LightCount;
    };

}