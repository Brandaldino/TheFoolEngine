#include "tfpch.h"
#include "ShadowRenderer.h"

#include "RenderGraph.h"

#include <glad/glad.h>

namespace TheFoolEngine
{

    void ShadowRenderer::Init(RenderGraph* graph)
    {
        m_DepthOnlyShader = Shader::Create("assets/shader/DepthOnlyShader.glsl");
        m_PointDepthShader = Shader::Create("assets/shader/PointShadowDepthShader.glsl");

        // Light UBO
        m_GPULightUBO = 0;
        glCreateBuffers(1, &m_GPULightUBO);
        glNamedBufferStorage(m_GPULightUBO, sizeof(LightGPUBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_GPULightUBO); // binding = 2

        TextureDesc dirSpotDesc;
        dirSpotDesc.Width = SHADOWMAP_SIZE;
        dirSpotDesc.Height = SHADOWMAP_SIZE;
        dirSpotDesc.Type = RenderTargetType::DepthArray;
        dirSpotDesc.LayerCount = MAX_SHADOW_LIGHTS;
        dirSpotDesc.IsTransient = true;
        m_ShadowFBOHandle = graph->CreateRenderTarget(dirSpotDesc, "DirectionalShadow");

        TextureDesc pointDesc;
        pointDesc.Width = 1024;
        pointDesc.Height = 1024;
        pointDesc.Type = RenderTargetType::CubeMapArray;
        pointDesc.LayerCount = MAX_SHADOW_LIGHTS;
        pointDesc.IsTransient = true;
        m_PointShadowHandle = graph->CreateRenderTarget(pointDesc, "PointShadow");
    }

    void ShadowRenderer::AddDirectionalLight(RenderContext& context, const DirectionLight& light, int shadowIndex)
    {
        GPULight& gpu = context.Lights.emplace_back();
        gpu.Position = glm::vec4(light.Direction * 1e6f, 0.0f);
        gpu.Direction = glm::vec4(light.Direction, 0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Directional);
        gpu.ShadowIndex = shadowIndex;
    }

    void ShadowRenderer::AddSpotLight(RenderContext& context, const SpotLight& light, int shadowIndex)
    {
        float innerCos = glm::cos(light.InnerAngle);
        float outerCos = glm::cos(light.OuterAngle);

        GPULight& gpu = context.Lights.emplace_back();
        gpu.Position = glm::vec4(light.Position, light.Range);
        gpu.Direction = glm::vec4(light.Direction, 0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(light.Range, innerCos, outerCos, (float)LightType::Spot);
        gpu.ShadowIndex = shadowIndex;
    }

    void ShadowRenderer::AddPointLight(RenderContext& context, const PointLight& light, int shadowIndex)
    {
        GPULight& gpu = context.Lights.emplace_back();
        gpu.Position = glm::vec4(light.Position, light.Range);
        gpu.Direction = glm::vec4(0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Point);
        gpu.ShadowIndex = shadowIndex;
    }

    void ShadowRenderer::SetGPULightFBO(RenderContext& context)
    {
        // light
        LightGPUBlock lightblock = {};
        lightblock.LightCount = (int32_t)context.Lights.size();
        for (int32_t i = 0; i < lightblock.LightCount && i < NR_LIGHTS; ++i)
        {
            lightblock.Lights[i].Position = context.Lights[i].Position;
            lightblock.Lights[i].Direction = context.Lights[i].Direction;
            lightblock.Lights[i].Color = context.Lights[i].Color;
            lightblock.Lights[i].Params = context.Lights[i].Params;
            lightblock.Lights[i].ShadowIndex = context.Lights[i].ShadowIndex;
        }
        glNamedBufferSubData(m_GPULightUBO, 0, sizeof(LightGPUBlock), &lightblock);
    }

    Ref<Shader> ShadowRenderer::GetDepthShader()
    {
        return m_DepthOnlyShader;
    }

    Ref<Shader> ShadowRenderer::GetPointDepthShader()
    {
        return m_PointDepthShader;
    }

    TextureHandle ShadowRenderer::GetShadowFBOHandle()
    {
        return m_ShadowFBOHandle;
    }

    TextureHandle ShadowRenderer::GetPointShadowHandle()
    {
        return m_PointShadowHandle;
    }

}