#include "tfpch.h"
#include "PBRRenderer.h"

#include "Shader.h"
#include "PerspectiveCameraController.h"
#include "RenderCommand.h"
#include "VertexArray.h"
#include "Pass/Pass.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{

    struct PBRRendererData
    {
        // shader
        Ref<Shader> Shader;

        // fallback textures
        Ref<Texture2D> DefaultWhite;
        Ref<Texture2D> DefaultNormal;
        Ref<Texture2D> DefaultGray;
        Ref<Texture2D> DefaultBlack;

        PBRMaterialTextureSet DefaultTexture;

        // Frame state
        GLuint LightUBO = 0;    // UBO handle

        PBRRenderState State;

        // Environment
        EnvironmentData Environment;

        // Shadow
        ShadowData Shadow;
        // PointLightShadow
        PointShadowData PointLightShadow;
    };

    static PBRRendererData s_Data;

    void PBRRenderer::Init()
    {
        s_Data.Shader = Shader::Create("assets/shader/PBRShader.glsl");

        s_Data.DefaultWhite = Texture2D::Create(1, 1);
        uint32_t whiteData = 0xffffffff;
        s_Data.DefaultWhite->SetData(&whiteData, sizeof(uint32_t));

        s_Data.DefaultBlack = Texture2D::Create(1, 1);
        uint32_t blackData = 0x00000000;
        s_Data.DefaultBlack->SetData(&blackData, sizeof(uint32_t));

        s_Data.DefaultNormal = Texture2D::Create(1, 1);
        uint32_t normalData = 0xffff8080; // (1,0.5,0.5,1) => decode to (0,0,1) in tangent space
        s_Data.DefaultNormal->SetData(&normalData, sizeof(uint32_t));

        s_Data.DefaultGray = Texture2D::Create(1, 1);
        uint32_t grayData = 0xFF7F7F7F; // (0.5,0.5,0.5,1) => Roughness=0.5, Metallic=0.5
        s_Data.DefaultGray->SetData(&grayData, sizeof(uint32_t));

        s_Data.DefaultTexture.AlbedoMap = s_Data.DefaultWhite;
        s_Data.DefaultTexture.NormalMap = s_Data.DefaultNormal;
        s_Data.DefaultTexture.MetallicRoughnessMap = s_Data.DefaultGray;
        s_Data.DefaultTexture.AOMap = s_Data.DefaultWhite;

        int32_t samplers[MAX_TEXTURE_SLOTS];
        for (int32_t i = 0; i < MAX_TEXTURE_SLOTS; ++i)
            samplers[i] = i;

        s_Data.Shader->Bind();
        s_Data.Shader->SetIntArray("u_Textures", samplers, MAX_TEXTURE_SLOTS);

        // Light UBO
        glCreateBuffers(1, &s_Data.LightUBO);
        glNamedBufferStorage(s_Data.LightUBO, sizeof(LightGPUBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, s_Data.LightUBO); // binding = 2

        // SkyBox
        s_Data.Environment.SkyboxShader = Shader::Create("assets/shader/SkyBoxShader.glsl");

        float skyboxVerts[] = {
            -1, 1,-1, -1,-1,-1,  1,-1,-1,
             1,-1,-1,  1, 1,-1, -1, 1,-1,
            -1,-1, 1, -1,-1,-1, -1, 1,-1,
            -1, 1,-1, -1, 1, 1, -1,-1, 1,
             1,-1,-1,  1,-1, 1,  1, 1, 1,
             1, 1, 1,  1, 1,-1,  1,-1,-1,
            -1,-1, 1, -1, 1, 1,  1, 1, 1,
             1, 1, 1,  1,-1, 1, -1,-1, 1,
            -1, 1,-1,  1, 1,-1,  1, 1, 1,
             1, 1, 1, -1, 1, 1, -1, 1,-1,
            -1,-1,-1, -1,-1, 1,  1,-1,-1,
             1,-1,-1, -1,-1, 1,  1,-1, 1,
        };

        s_Data.Environment.SkyboxCubeVAO = VertexArray::Create();
        auto skyVBO = VertexBuffer::Create(skyboxVerts, sizeof(skyboxVerts));
        skyVBO->SetLayout(
            {
                { ShaderDataType::Float3, "a_Position" }
            });
        s_Data.Environment.SkyboxCubeVAO->AddVertexBuffer(skyVBO);

        // Shadow
        FrameBufferSpecification shadowSpec;
        shadowSpec.Width = 1024;
        shadowSpec.Height = 1024;
        shadowSpec.DepthOnly = true;
        shadowSpec.LayerCount = MAX_SHADOW_LIGHTS;
        s_Data.Shadow.ShadowFBO = FrameBuffer::Create(shadowSpec);
        s_Data.Shadow.DepthOnlyShader = Shader::Create("assets/shader/DepthOnlyShader.glsl");

        s_Data.PointLightShadow.DepthShader = Shader::Create("assets/shader/PointShadowDepthShader.glsl");
        s_Data.PointLightShadow.DepthMap = PointShadowMap::Create(SHADOWMAP_SIZE, MAX_SHADOW_LIGHTS);
    }

    void PBRRenderer::Shutdown()
    {
    }

    void PBRRenderer::ResetRendererState()
    {
        TF_PROFILE_FUNCTION();

        ResetStats();

        s_Data.Shadow.LightViewProjections.clear();
        s_Data.PointLightShadow.Count = 0;

        s_Data.Shader->Bind();
        s_Data.DefaultWhite->Bind(1);
        s_Data.DefaultNormal->Bind(2);
        s_Data.DefaultGray->Bind(3);
        s_Data.DefaultWhite->Bind(4);
    }
    
    void PBRRenderer::AddLight(RenderContext& context, const DirectionLight& light)
    {
        AddLight(context, light, -1);
    }

    void PBRRenderer::AddLight(RenderContext& context, const DirectionLight& light, int shadowIndex)
    {
        GPULight& gpu = context.Lights.emplace_back();
        gpu.Position = glm::vec4(light.Direction * 1e6f, 0.0f);
        gpu.Direction = glm::vec4(light.Direction, 0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Directional);
        gpu.ShadowIndex = shadowIndex;
    }

    void PBRRenderer::AddLight(RenderContext& context, const PointLight& light)
    {
        AddLight(context, light, -1);
    }

    void PBRRenderer::AddLight(RenderContext& context, const PointLight& light, int shadowIndex)
    {
        GPULight& gpu = context.Lights.emplace_back();
        gpu.Position = glm::vec4(light.Position, light.Range);
        gpu.Direction = glm::vec4(0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Point);
        gpu.ShadowIndex = shadowIndex;
    }

    void PBRRenderer::AddLight(RenderContext& context, const SpotLight& light)
    {
        AddLight(context, light, -1);
    }

    void PBRRenderer::AddLight(RenderContext& context, const SpotLight& light, int shadowIndex)
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

    void PBRRenderer::Render(const RenderContext& context)
    {
        TF_PROFILE_FUNCTION();

        s_Data.Shader->Bind();

        // Shadow
        RenderCommand::BindArrayTexture(s_Data.Shadow.ShadowFBO->GetDepthArrayTextureID(), 8);
        s_Data.Shader->SetInt("u_ShadowMaps", 8);
        // PointLightShadow
        if (s_Data.PointLightShadow.DepthMap)
        {
            glBindTextureUnit(9, s_Data.PointLightShadow.DepthMap->GetRendererID());
            s_Data.Shader->SetInt("u_PointShadowMaps", 9);

            std::vector<float> farPlanes(MAX_SHADOW_LIGHTS, 100.0f);
            for (uint32_t i = 0; i < s_Data.PointLightShadow.Count; ++i)
                farPlanes[i] = s_Data.PointLightShadow.Lights[i].FarPlane;
            s_Data.Shader->SetFloatArray("u_PointShadowFarPlanes", farPlanes);
        }
        std::vector<glm::mat4> shadowLightsViewProjs;
        for (int i = 0; i < (int)s_Data.Shadow.LightViewProjections.size() && i < MAX_SHADOW_LIGHTS; ++i)
            shadowLightsViewProjs.push_back(s_Data.Shadow.LightViewProjections[i]);
        s_Data.Shader->SetMat4Array("u_ShadowMatrices", shadowLightsViewProjs);

        // camera
        s_Data.Shader->SetMat4("u_View", context.Camera.ViewMatrix);
        s_Data.Shader->SetMat4("u_Projection", context.Camera.ProjectionMatrix);
        s_Data.Shader->SetFloat3("u_CameraPos", context.Camera.Position);

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
        glNamedBufferSubData(s_Data.LightUBO, 0, sizeof(LightGPUBlock), &lightblock);

        // entity
        for (auto& proxy : context.Renderables)
        {
            if (!proxy.Visible)
                continue;

            auto& modelData = proxy.Model->GetModelData();
            auto& vas = proxy.Model->GetVertexArray();
            auto& meshes = modelData.Meshes;
            auto& texSets = modelData.Textures;

            for (std::size_t i = 0; i < vas.size(); ++i)
            {
                // Per-mesh transform
                glm::mat4 model = proxy.Transform * meshes[i].NodeTransform;
                s_Data.Shader->SetMat4("u_Model", model);

                // Per-mesh textures (override fallback if exists)
                auto& texSet = texSets[meshes[i].MaterialIndex];
                if (texSet.AlbedoMap)               texSet.AlbedoMap->Bind(1);
                if (texSet.NormalMap)               texSet.NormalMap->Bind(2);
                if (texSet.MetallicRoughnessMap)    texSet.MetallicRoughnessMap->Bind(3);
                if (texSet.AOMap)                    texSet.AOMap->Bind(4);

                // Per-mesh factors
                s_Data.Shader->SetFloat3("u_AlbedoFactor", texSet.AlbedoFactor);
                s_Data.Shader->SetFloat("u_MetallicFactor", texSet.MetallicFactor);
                s_Data.Shader->SetFloat("u_RoughnessFactor", texSet.RoughnessFactor);
                s_Data.Shader->SetFloat("u_AOStrength", texSet.AOStrength);

                // Draw
                vas[i]->Bind();
                RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());

                s_Data.State.DrawCalls++;
            }
            s_Data.State.MeshCount += (uint32_t)vas.size();
        }

        // Skybox
        if (s_Data.Environment.Skybox && s_Data.Environment.SkyboxCubeVAO)
        {
            s_Data.Environment.SkyboxShader->Bind();
            s_Data.Environment.SkyboxShader->SetMat4("u_Projection", context.Camera.ProjectionMatrix);
            glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(context.Camera.ViewMatrix));
            s_Data.Environment.SkyboxShader->SetMat4("u_View", viewNoTranslate);
            s_Data.Environment.Skybox->Bind(0);
            s_Data.Environment.SkyboxShader->SetInt("u_Skybox", 0);

            RenderCommand::SetDepthFunc(RendererAPI::DepthFunc::LessEqual);
            s_Data.Environment.SkyboxCubeVAO->Bind();
            RenderCommand::DrawArrays(RendererAPI::DrawMode::Triangles, 36);
            RenderCommand::SetDepthFunc(RendererAPI::DepthFunc::Less);
        }
    }

    void PBRRenderer::ResetStats()
    {
        s_Data.State.DrawCalls = 0;
        s_Data.State.MeshCount = 0;
    }

    PBRRenderState PBRRenderer::GetStats()
    {
        return s_Data.State;
    }

    std::int32_t PBRRenderer::GetLightsCount(const RenderContext& context)
    {
        return (std::int32_t)context.Lights.size();
    }

    void PBRRenderer::DefaultTextureFill(Ref<PBRModel> model)
    {
        model->CheckTexture(s_Data.DefaultTexture);
    }

    void PBRRenderer::SetSkybox(const Ref<CubeMap> skybox)
    {
        s_Data.Environment.Skybox = skybox;
    }

    void PBRRenderer::SetEnvironmentMap(const Ref<CubeMap> irradiance, const Ref<CubeMap> prefilter, const Ref<Texture2D> brdfLUT)
    {
        s_Data.Environment.IrradianceMap = irradiance;
        s_Data.Environment.PrefilterMap = prefilter;
        s_Data.Environment.BRDFLUT = brdfLUT;
    }

    int PBRRenderer::SetShadowLight(const glm::vec3& lightDir, float orthoSize, float nearPlane, float farPlane)
    {
        glm::vec3 sceneCenter = glm::vec3(0.0f);    // TODO: Use the origin temporarily; the scene bounding box can be replaced later.
        glm::vec3 lightPos = sceneCenter - lightDir * 50.0f;

        glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);

        int index = (int)s_Data.Shadow.LightViewProjections.size();
        s_Data.Shadow.LightViewProjections.push_back(lightProj * lightView);
        return index;
    }

    int PBRRenderer::SetShadowLight(RenderContext& context, const glm::vec3& lightDir, float orthoSize, float nearPlane, float farPlane)
    {
        glm::vec3 sceneCenter = glm::vec3(0.0f);    // TODO: Use the origin temporarily; the scene bounding box can be replaced later.
        glm::vec3 lightPos = sceneCenter - lightDir * 50.0f;

        glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);

        int index = (int)s_Data.Shadow.LightViewProjections.size();
        s_Data.Shadow.LightViewProjections.push_back(lightProj * lightView);
        return index;
    }

    int PBRRenderer::SetSpotShadowLight(const glm::vec3& position, const glm::vec3& direction, float fovDeg, float nearPlane, float farPlane)
    {
        glm::vec3 up = (glm::abs(glm::dot(direction, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
            ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 lightView = glm::lookAt(position, position + direction, up);
        glm::mat4 lightProj = glm::perspective(glm::radians(fovDeg), 1.0f, nearPlane, farPlane);

        int index = (int)s_Data.Shadow.LightViewProjections.size();
        s_Data.Shadow.LightViewProjections.push_back(lightProj * lightView);
        return index;
    }

    int PBRRenderer::SetPointShadowLight(const glm::vec3& position, float nearPlane, float farPlane)
    {
        if (s_Data.PointLightShadow.Count >= MAX_SHADOW_LIGHTS)
            return -1;   // Shadow slot limit reached

        uint32_t index = s_Data.PointLightShadow.Count++;
        auto& light = s_Data.PointLightShadow.Lights[index];
        light.LightPosition = position;
        light.FarPlane = farPlane;
        light.ShadowProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

        glm::mat4* v = light.ShadowViews;
        v[0] = glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        v[1] = glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        v[2] = glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        v[3] = glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        v[4] = glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        v[5] = glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

        return (int)index;
    }

    void PBRRenderer::RenderShadowPass(const RenderContext& context)
    {
        uint32_t layerCount = (uint32_t)s_Data.Shadow.LightViewProjections.size();
        if (layerCount == 0)
            return;

        s_Data.Shadow.ShadowFBO->Bind();
        s_Data.Shadow.DepthOnlyShader->Bind();

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);

        for (uint32_t layer = 0; layer < layerCount; ++layer)
        {
            if (layer >= MAX_SHADOW_LIGHTS)
                break;

            s_Data.Shadow.ShadowFBO->AttachLayer(layer);
            RenderCommand::SetClearColor({ 1.0f,1.0f, 1.0f, 1.0f });
            RenderCommand::Clear();
            s_Data.Shadow.DepthOnlyShader->SetMat4("u_LightViewProjection", s_Data.Shadow.LightViewProjections[layer]);

            for (auto& proxy : context.Renderables)
            {
                if (!proxy.Visible)
                    continue;

                auto& modelData = proxy.Model->GetModelData();
                auto& vas = proxy.Model->GetVertexArray();
                auto& meshes = modelData.Meshes;

                for (std::size_t i = 0; i < vas.size(); ++i)
                {
                    glm::mat4 model = proxy.Transform * meshes[i].NodeTransform;
                    s_Data.Shadow.DepthOnlyShader->SetMat4("u_Model", model);
                    vas[i]->Bind();
                    RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
                }
            }
        }

        s_Data.Shadow.ShadowFBO->UnBind();
    }

    void PBRRenderer::RenderPointShadowPass(const RenderContext& context)
    {
        if (!s_Data.PointLightShadow.DepthMap)
            return;

        s_Data.PointLightShadow.DepthMap->Bind();
        s_Data.PointLightShadow.DepthShader->Bind();

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);
        RenderCommand::SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        RenderCommand::Clear();

        for (uint32_t lightIndex = 0; lightIndex < s_Data.PointLightShadow.Count; ++lightIndex)
        {
            const auto& light = s_Data.PointLightShadow.Lights[lightIndex];

            for (int face = 0; face < 6; ++face)
            {
                s_Data.PointLightShadow.DepthMap->BindFace(lightIndex, face);
                RenderCommand::Clear();
                s_Data.PointLightShadow.DepthShader->SetMat4("u_LightViewProjection",
                    light.ShadowProj * light.ShadowViews[face]);
                s_Data.PointLightShadow.DepthShader->SetFloat3("u_LightPos", light.LightPosition);
                s_Data.PointLightShadow.DepthShader->SetFloat("u_FarPlane", light.FarPlane);

                // Traverse renderables to draw depth
                for (auto& proxy : context.Renderables)
                {
                    if (!proxy.Visible)
                        continue;

                    auto& modelData = proxy.Model->GetModelData();
                    auto& vas = proxy.Model->GetVertexArray();
                    auto& meshes = modelData.Meshes;
                    for (std::size_t i = 0; i < vas.size(); ++i)
                    {
                        glm::mat4 model = proxy.Transform * meshes[i].NodeTransform;
                        s_Data.PointLightShadow.DepthShader->SetMat4("u_Model", model);
                        vas[i]->Bind();
                        RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
                    }
                }
            }
        }

        s_Data.PointLightShadow.DepthMap->Unbind();
    }

    // =====================================================================================
    Ref<Shader> PBRRenderer::GetPBRShader()
    {
        return s_Data.Shader;
    }

    const PBRMaterialTextureSet& PBRRenderer::GetDefaultTexture()
    {
        return s_Data.DefaultTexture;
    }

    uint32_t PBRRenderer::GetLightUBO()
    {
        return s_Data.LightUBO;
    }

    const EnvironmentData& PBRRenderer::GetEnvironment()
    {
        return s_Data.Environment;
    }

    Ref<FrameBuffer> PBRRenderer::GetShadowFBO()
    {
        return s_Data.Shadow.ShadowFBO;
    }

    Ref<Shader> PBRRenderer::GetDepthOnlyShader()
    {
        return s_Data.Shadow.DepthOnlyShader;
    }

    const std::vector<glm::mat4>& PBRRenderer::GetShadowViewProjections()
    {
        return s_Data.Shadow.LightViewProjections;
    }

    Ref<Shader> PBRRenderer::GetPointShadowDepthShader()
    {
        return s_Data.PointLightShadow.DepthShader;
    }

    Ref<PointShadowMap> PBRRenderer::GetPointShadowMap()
    {
        return s_Data.PointLightShadow.DepthMap;
    }

    const PointShadowData& PBRRenderer::GetPointShadowData()
    {
        return s_Data.PointLightShadow;
    }

    // =====================================================================================
}
