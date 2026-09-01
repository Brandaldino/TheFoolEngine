#pragma once

#include "PointShadowMap.h"
#include "FrameBuffer.h"
#include "Shader.h"

namespace TheFoolEngine
{
    constexpr uint32_t MAX_SHADOW_LIGHTS = 8;
    constexpr uint32_t SHADOWMAP_SIZE = 1024;
    constexpr uint32_t NR_LIGHTS = 10;

    struct ShadowData
    {
        Ref<FrameBuffer> ShadowFBO;
        Ref<Shader> DepthOnlyShader;
        std::vector<glm::mat4> LightViewProjections;
    };

    struct PointLightShadowData
    {
        glm::vec3 LightPosition;
        glm::mat4 ShadowViews[6];
        glm::mat4 ShadowProj;
        float FarPlane;
    };

    struct PointShadowData
    {
        Ref<Shader> DepthShader;
        Ref<PointShadowMap> DepthMap;
        PointLightShadowData Lights[MAX_SHADOW_LIGHTS];
        uint32_t Count = 0;
    };

    struct LightGPUBlock
    {
        GPULight Lights[NR_LIGHTS];
        int32_t LightCount;
    };

}