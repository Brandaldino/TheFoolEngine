#include "tfpch.h"
#include "PBRRenderer.h"

#include "Shader.h"
#include "PerspectiveCameraController.h"
#include "RenderCommand.h"
#include "VertexArray.h"

#include <glad/glad.h>

namespace TheFoolEngine
{
    constexpr uint32_t NR_LIGHTS = 10;
    constexpr uint32_t MAX_TEXTURE_SLOTS = 32;

    struct EnvironmentData
    {
        Ref<CubeMap> Skybox;
        Ref<Shader> SkyboxShader;
        Ref<VertexArray> SkyboxCubeVAO;
        Ref<CubeMap> IrradianceMap;
        Ref<CubeMap> PrefilterMap;
        Ref<Texture2D> BRDFLUT;
    };

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
        std::vector<PBRRenderProxy> Renderables;
        std::vector<GPULight> GPULights;
        GLuint LightUBO = 0;    // UBO handle
        CameraData Camera;

        PBRRenderState State;

        // Environment
        EnvironmentData Environment;
    };

    struct LightGPUBlock
    {
        GPULight Lights[NR_LIGHTS];
        int32_t LightCount;
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

    }

    void PBRRenderer::Shutdown()
    {
    }

    void PBRRenderer::ResetRendererState()
    {
        ResetStats();

        s_Data.Renderables.clear();
        s_Data.GPULights.clear();

        s_Data.Shader->Bind();
        s_Data.DefaultWhite->Bind(1);
        s_Data.DefaultNormal->Bind(2);
        s_Data.DefaultGray->Bind(3);
        s_Data.DefaultWhite->Bind(4);
    }

    void PBRRenderer::Register(const PBRRenderProxy& proxy)
    {
        s_Data.Renderables.push_back(proxy);
    }

    void PBRRenderer::SetCamera(const CameraData& camera)
    {
        s_Data.Camera.ViewMatrix = camera.ViewMatrix;
        s_Data.Camera.ProjectionMatrix = camera.ProjectionMatrix;
        s_Data.Camera.Position = camera.Position;
    }

    void PBRRenderer::AddLight(const DirectionLight& light)
    {
        GPULight& gpu = s_Data.GPULights.emplace_back();
        gpu.Position = glm::vec4(light.Direction * 1e6f, 0.0f);
        gpu.Direction = glm::vec4(light.Direction, 0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Directional);
    }

    void PBRRenderer::AddLight(const PointLight& light)
    {
        GPULight& gpu = s_Data.GPULights.emplace_back();
        gpu.Position = glm::vec4(light.Position, light.Range);
        gpu.Direction = glm::vec4(0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(0.0f, 0.0f, 0.0f, (float)LightType::Point);
    }

    void PBRRenderer::AddLight(const SpotLight& light)
    {
        float innerCos = glm::cos(light.InnerAngle);
        float outerCos = glm::cos(light.OuterAngle);

        GPULight& gpu = s_Data.GPULights.emplace_back();
        gpu.Position = glm::vec4(light.Position, light.Range);
        gpu.Direction = glm::vec4(light.Direction, 0.0f);
        gpu.Color = glm::vec4(light.Color, light.Intensity);
        gpu.Params = glm::vec4(light.Range, innerCos, outerCos, (float)LightType::Spot);
    }

    void PBRRenderer::Render()
    {
        TF_PROFILE_FUNCTION();

        s_Data.Shader->Bind();

        // camera
        s_Data.Shader->SetMat4("u_View", s_Data.Camera.ViewMatrix);
        s_Data.Shader->SetMat4("u_Projection", s_Data.Camera.ProjectionMatrix);
        s_Data.Shader->SetFloat3("u_CameraPos", s_Data.Camera.Position);

        // light
        LightGPUBlock lightblock = {};
        lightblock.LightCount = (int32_t)s_Data.GPULights.size();
        for (int32_t i = 0; i < lightblock.LightCount && i < NR_LIGHTS; ++i)
        {
            lightblock.Lights[i].Position = s_Data.GPULights[i].Position;
            lightblock.Lights[i].Direction = s_Data.GPULights[i].Direction;
            lightblock.Lights[i].Color = s_Data.GPULights[i].Color;
            lightblock.Lights[i].Params = s_Data.GPULights[i].Params;
        }
        glNamedBufferSubData(s_Data.LightUBO, 0, sizeof(LightGPUBlock), &lightblock);

        // entity
        for (auto& proxy : s_Data.Renderables)
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
            s_Data.Environment.SkyboxShader->SetMat4("u_Projection", s_Data.Camera.ProjectionMatrix);
            glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(s_Data.Camera.ViewMatrix));
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

    std::int32_t PBRRenderer::GetLightsCount()
    {
        return (std::int32_t)s_Data.GPULights.size();
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

}
