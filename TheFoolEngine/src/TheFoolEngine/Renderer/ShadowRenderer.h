#pragma once

#include "Shader.h"

#include "Pass/Pass.h"
#include "ShadowTypes.h"
#include "Light.h"

#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{
    class RenderGraph;

    class ShadowRenderer
    {
    public:
        void Init(RenderGraph* graph);

        // data upload
        void AddDirectionalLight(RenderContext& context, const DirectionLight& light, int shadowIndex = -1);
        void AddSpotLight(RenderContext& context, const SpotLight& light, int shadowIndex = -1);
        void AddPointLight(RenderContext& context, const PointLight& light, int shadowIndex = -1);
        void SetGPULightFBO(RenderContext& context);

        // get resource
        Ref<Shader> GetDepthShader();
        Ref<Shader> GetPointDepthShader();

        TextureHandle GetShadowFBOHandle();
        TextureHandle GetPointShadowHandle();
    private:
        Ref<Shader> m_DepthOnlyShader;
        Ref<Shader> m_PointDepthShader;

        uint32_t m_GPULightUBO;

        TextureHandle m_ShadowFBOHandle;
        TextureHandle m_PointShadowHandle;
    };

}