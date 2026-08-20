#include "tfpch.h"
#include "MainPass.h"

#include "../PBRRenderer.h"
#include "../RenderCommand.h"
#include "../Shader.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{

    std::vector<TextureHandle>& MainPass::GetOutputs()
    {
        return m_Outputs;
    }

    void MainPass::SetTarget(Ref<FrameBuffer> target, Ref<Shader> shader)
    {
        m_OutputFBO = target;
        m_Shader = shader;

        TextureHandle out;
        out.Desc.Width = target->GetSpecification().Width;
        out.Desc.Height = target->GetSpecification().Height;
        out.Desc.Format = target->GetSpecification().FrameBufferFormat;
        out.Name = "MainPassOutput";

        m_Outputs.clear();
        m_Outputs.push_back(out);
    }

    void MainPass::Execute(RenderContext& ctx)
    {
        TF_PROFILE_FUNCTION();

        m_OutputFBO->Bind();
        m_Shader->Bind();

        // Shadow
        RenderCommand::BindArrayTexture(PBRRenderer::GetShadowFBO()->GetDepthArrayTextureID(), 8);
        m_Shader->SetInt("u_ShadowMaps", 8);
        // PointLightShadow
        if (PBRRenderer::GetPointShadowMap())
        {
            glBindTextureUnit(9, PBRRenderer::GetPointShadowMap()->GetRendererID());
            m_Shader->SetInt("u_PointShadowMaps", 9);

            std::vector<float> farPlanes(MAX_SHADOW_LIGHTS, 100.0f);
            for (uint32_t i = 0; i < PBRRenderer::GetPointShadowData().Count; ++i)
                farPlanes[i] = PBRRenderer::GetPointShadowData().Lights[i].FarPlane;
            m_Shader->SetFloatArray("u_PointShadowFarPlanes", farPlanes);
        }
        std::vector<glm::mat4> shadowLightsViewProjs;
        for (int i = 0; i < (int)PBRRenderer::GetShadowViewProjections().size() && i < MAX_SHADOW_LIGHTS; ++i)
            shadowLightsViewProjs.push_back(PBRRenderer::GetShadowViewProjections()[i]);
        m_Shader->SetMat4Array("u_ShadowMatrices", shadowLightsViewProjs);

        // camera
        m_Shader->SetMat4("u_View", ctx.Camera.ViewMatrix);
        m_Shader->SetMat4("u_Projection", ctx.Camera.ProjectionMatrix);
        m_Shader->SetFloat3("u_CameraPos", ctx.Camera.Position);

        // light
        LightGPUBlock lightblock = {};
        lightblock.LightCount = (int32_t)ctx.Lights.size();
        for (int32_t i = 0; i < lightblock.LightCount && i < NR_LIGHTS; ++i)
        {
            lightblock.Lights[i].Position = ctx.Lights[i].Position;
            lightblock.Lights[i].Direction = ctx.Lights[i].Direction;
            lightblock.Lights[i].Color = ctx.Lights[i].Color;
            lightblock.Lights[i].Params = ctx.Lights[i].Params;
            lightblock.Lights[i].ShadowIndex = ctx.Lights[i].ShadowIndex;
        }
        glNamedBufferSubData(PBRRenderer::GetLightUBO(), 0, sizeof(LightGPUBlock), &lightblock);

        // entity
        for (auto& proxy : ctx.Renderables)
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
                m_Shader->SetMat4("u_Model", model);

                // Per-mesh textures (override fallback if exists)
                auto& texSet = texSets[meshes[i].MaterialIndex];
                if (texSet.AlbedoMap)               texSet.AlbedoMap->Bind(1);
                if (texSet.NormalMap)               texSet.NormalMap->Bind(2);
                if (texSet.MetallicRoughnessMap)    texSet.MetallicRoughnessMap->Bind(3);
                if (texSet.AOMap)                    texSet.AOMap->Bind(4);

                // Per-mesh factors
                m_Shader->SetFloat3("u_AlbedoFactor", texSet.AlbedoFactor);
                m_Shader->SetFloat("u_MetallicFactor", texSet.MetallicFactor);
                m_Shader->SetFloat("u_RoughnessFactor", texSet.RoughnessFactor);
                m_Shader->SetFloat("u_AOStrength", texSet.AOStrength);

                // Draw
                vas[i]->Bind();
                RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
            }
        }

        // Skybox
        if (PBRRenderer::GetEnvironment().Skybox && PBRRenderer::GetEnvironment().SkyboxCubeVAO)
        {
            PBRRenderer::GetEnvironment().SkyboxShader->Bind();
            PBRRenderer::GetEnvironment().SkyboxShader->SetMat4("u_Projection", ctx.Camera.ProjectionMatrix);
            glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(ctx.Camera.ViewMatrix));
            PBRRenderer::GetEnvironment().SkyboxShader->SetMat4("u_View", viewNoTranslate);
            PBRRenderer::GetEnvironment().Skybox->Bind(0);
            PBRRenderer::GetEnvironment().SkyboxShader->SetInt("u_Skybox", 0);

            RenderCommand::SetDepthFunc(RendererAPI::DepthFunc::LessEqual);
            PBRRenderer::GetEnvironment().SkyboxCubeVAO->Bind();
            RenderCommand::DrawArrays(RendererAPI::DrawMode::Triangles, 36);
            RenderCommand::SetDepthFunc(RendererAPI::DepthFunc::Less);
        }
    }

}