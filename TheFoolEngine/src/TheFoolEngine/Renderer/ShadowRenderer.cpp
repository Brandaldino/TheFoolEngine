#include "tfpch.h"
#include "ShadowRenderer.h"

#include "RenderGraph.h"

#include <glad/glad.h>

namespace TheFoolEngine
{

    void ShadowRenderer::Init(RenderGraph* graph)
    {
        FrameBufferSpecification shadowSpec;
        shadowSpec.Width = 1024;
        shadowSpec.Height = 1024;
        shadowSpec.DepthOnly = true;
        shadowSpec.LayerCount = MAX_SHADOW_LIGHTS;
        m_ShadowData.ShadowFBO = FrameBuffer::Create(shadowSpec);
        m_ShadowData.DepthOnlyShader = Shader::Create("assets/shader/DepthOnlyShader.glsl");

        // m_PointShadowData.DepthMap = PointShadowMap::Create(SHADOWMAP_SIZE, MAX_SHADOW_LIGHTS);
        m_PointShadowData.DepthShader = Shader::Create("assets/shader/PointShadowDepthShader.glsl");

        // Light UBO
        m_GPULightUBO = 0;
        glCreateBuffers(1, &m_GPULightUBO);
        glNamedBufferStorage(m_GPULightUBO, sizeof(LightGPUBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_GPULightUBO); // binding = 2

        // Texture Handle
        TextureDesc dirSpotDesc;
        dirSpotDesc.Width = 1024;
        dirSpotDesc.Height = 1024;
        dirSpotDesc.Type = RenderTargetType::DepthArray;
        dirSpotDesc.LayerCount = MAX_SHADOW_LIGHTS;
        m_ShadowFBOHandle = graph->CreateRenderTarget(dirSpotDesc, "DirectionalShadow");

        TextureDesc pointDesc;
        pointDesc.Width = 1024;
        pointDesc.Height = 1024;
        pointDesc.Type = RenderTargetType::CubeMapArray;
        pointDesc.LayerCount = MAX_SHADOW_LIGHTS;
        m_PointShadowHandle = graph->CreateRenderTarget(pointDesc, "PointShadow");
    }

    void ShadowRenderer::Reset()
    {
        m_ShadowData.LightViewProjections.clear();
        m_PointShadowData.Count = 0;
    }

    int ShadowRenderer::SetDirectionalLight(const glm::vec3& dir, float ortho, float nearPlane, float farPlane)
    {
        glm::vec3 sceneCenter = glm::vec3(0.0f);    // TODO: Use the origin temporarily; the scene bounding box can be replaced later.
        glm::vec3 lightPos = sceneCenter - dir * 50.0f;

        glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 lightProj = glm::ortho(-ortho, ortho, -ortho, ortho, nearPlane, farPlane);

        int index = (int)m_ShadowData.LightViewProjections.size();
        m_ShadowData.LightViewProjections.push_back(lightProj * lightView);
        return index;
    }

    int ShadowRenderer::SetSpotLight(const glm::vec3& pos, const glm::vec3& dir, float fov, float nearPlane, float farPlane)
    {
        glm::vec3 up = (glm::abs(glm::dot(dir, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
            ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 lightView = glm::lookAt(pos, pos + dir, up);
        glm::mat4 lightProj = glm::perspective(glm::radians(fov), 1.0f, nearPlane, farPlane);

        int index = (int)m_ShadowData.LightViewProjections.size();
        m_ShadowData.LightViewProjections.push_back(lightProj * lightView);
        return index;
    }

    int ShadowRenderer::SetPointLight(const glm::vec3& pos, float nearPlane, float farPlane)
    {
        if (m_PointShadowData.Count >= MAX_SHADOW_LIGHTS)
            return -1;   // Shadow slot limit reached

        uint32_t index = m_PointShadowData.Count++;
        auto& light = m_PointShadowData.Lights[index];
        light.LightPosition = pos;
        light.FarPlane = farPlane;
        light.ShadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

        glm::mat4* v = light.ShadowViews;
        v[0] = glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        v[1] = glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        v[2] = glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        v[3] = glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        v[4] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        v[5] = glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

        return (int)index;
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

    Ref<Shader> ShadowRenderer::GetDepthShader()
    {
        return m_ShadowData.DepthOnlyShader;
    }

    Ref<Shader> ShadowRenderer::GetPointDepthShader()
    {
        return m_PointShadowData.DepthShader;
    }

    const std::vector<glm::mat4>& ShadowRenderer::GetViewProjections()
    {
        return m_ShadowData.LightViewProjections;
    }

    const PointShadowData& ShadowRenderer::GetPointShadowData()
    {
        return m_PointShadowData;
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