#include "tfpch.h"
#include "PBRRenderer.h"

#include "Shader.h"
#include "PerspectiveCameraController.h"
#include "RenderCommand.h"

#include <glad/glad.h>

namespace TheFoolEngine
{
    constexpr uint32_t NR_LIGHTS = 10;
    constexpr uint32_t MAX_TEXTURE_SLOTS = 32;

    struct PBRRendererData
    {
        // shader
        Ref<Shader> Shader;

        // fallback textures
        Ref<Texture2D> DefaultWhite;
        Ref<Texture2D> DefaultNormal;
        Ref<Texture2D> DefaultGray;
        Ref<Texture2D> DefaultBlack;

        // Frame state
        std::vector<PBRRenderProxy> Renderables;
        std::vector<Light> Lights;
        GLuint LightUBO = 0;    // UBO handle
        CameraData Camera;

        PBRRenderState State;
    };

    struct LightGPUBlock
    {
        glm::vec4 PositionType[NR_LIGHTS];     // xyz = pos, w = type
        glm::vec4 ColorIntensity[NR_LIGHTS];   // xyz = color, w = intensity
        int32_t LightCount;
    };

    static PBRRendererData s_Data;

    void PBRRenderer::Init()
    {
        s_Data.Shader = Shader::Create("assets/shaders/PBRShader.glsl");

        s_Data.DefaultWhite = Texture2D::Create(1, 1);
        uint32_t whiteData = 0xffffffff;
        s_Data.DefaultWhite->SetData(&whiteData, sizeof(uint32_t));

        s_Data.DefaultBlack = Texture2D::Create(1, 1);
        uint32_t blackData = 0x00000000;
        s_Data.DefaultBlack->SetData(&blackData, sizeof(uint32_t));

        s_Data.DefaultNormal = Texture2D::Create(1, 1);
        uint32_t normalData = 0xffff8080; // (1,0.5,0.5,1) → decode to (0,0,1) in tangent space
        s_Data.DefaultNormal->SetData(&normalData, sizeof(uint32_t));

        s_Data.DefaultGray = Texture2D::Create(1, 1);
        uint32_t grayData = 0xFF7F7F7F; // (0.5,0.5,0.5,1) → Roughness=0.5, Metallic=0.5
        s_Data.DefaultGray->SetData(&grayData, sizeof(uint32_t));

        int32_t samplers[MAX_TEXTURE_SLOTS];
        for (int32_t i = 0; i < MAX_TEXTURE_SLOTS; ++i)
            samplers[i] = i;

        s_Data.Shader->Bind();
        s_Data.Shader->SetIntArray("u_Textures", samplers, MAX_TEXTURE_SLOTS);

        // Light UBO
        glCreateBuffers(1, &s_Data.LightUBO);
        glNamedBufferStorage(s_Data.LightUBO, sizeof(LightGPUBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, s_Data.LightUBO); // binding = 2
    }

    void PBRRenderer::Shutdown()
    {
    }

    void PBRRenderer::ResetRendererState()
    {
        ResetStats();

        s_Data.Renderables.clear();
        s_Data.Lights.clear();

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

    void PBRRenderer::SetLight(const Light& light)
    {
        s_Data.Lights.push_back(light);
    }

    void PBRRenderer::SetCamera(const CameraData& camera)
    {
        s_Data.Camera.ViewMatrix = camera.ViewMatrix;
        s_Data.Camera.ProjectionMatrix = camera.ProjectionMatrix;
        s_Data.Camera.Position = camera.Position;
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
        lightblock.LightCount = (int32_t)s_Data.Lights.size();
        for (int32_t i = 0; i < lightblock.LightCount && i < NR_LIGHTS; ++i)
        {
            lightblock.PositionType[i] = glm::vec4(s_Data.Lights[i].Position, (float)s_Data.Lights[i].Type);
            lightblock.ColorIntensity[i] = glm::vec4(s_Data.Lights[i].Color, s_Data.Lights[i].Intensity);
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
                model = glm::mat4(1.0f);
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



}
