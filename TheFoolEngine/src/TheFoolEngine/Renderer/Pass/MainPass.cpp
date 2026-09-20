#include "tfpch.h"
#include "MainPass.h"

#include "../PBRRenderer.h"
#include "../PointShadowMap.h"
#include "../RenderCommand.h"
#include "../Shader.h"
#include "../RenderGraph.h"
#include "../LightPacker.h"
#include "../BatchSystem/BatchBuilder.h"
#include "../InstanceRenderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine
{
    MainPass::MainPass(Ref<Shader> shader)
        :m_Shader(shader)
    {
        m_GPULightUBO = 0;

        glCreateBuffers(1, &m_GPULightUBO);
        glNamedBufferStorage(m_GPULightUBO, sizeof(LightGPUBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_GPULightUBO);

        m_InstanceRenderer = CreateScope<InstanceRenderer>(s_MaxInstances);
    }

    void MainPass::SetOutput(TextureHandle& output)
    {
        m_Output = output;
    }

    std::vector<TextureHandle>& MainPass::GetInputs()
    {
        m_Inputs = { m_InputShadow, m_InputPointShadow };
        return m_Inputs;
    }

    std::vector<TextureHandle>& MainPass::GetOutputs()
    {
        m_Outputs = { m_Output };
        return m_Outputs;
    }

    void MainPass::Execute(RenderContext& ctx)
    {
        TF_PROFILE_FUNCTION();

        // light update
        LightPacker::SetGPULightFBO(m_GPULightUBO, ctx.Lights);

        m_Shader->Bind();

        BatchBuilder batchBuilder;
        for (auto& output : m_Outputs)
        {
            ctx.RenderGraph->GetFrameBuffer(output)->Bind();
            RenderCommand::SetClearColor({ 0.1f,0.1f, 0.1f, 1.0f });
            RenderCommand::Clear();

            // Shadow
            RenderCommand::BindArrayTexture(ctx.RenderGraph->GetFrameBuffer(m_InputShadow)->GetDepthArrayTextureID(), 8);
            m_Shader->SetInt("u_ShadowMaps", 8);
            // PointLightShadow
            if (ctx.RenderGraph->GetPointShadowMap(m_InputPointShadow))
            {
                glBindTextureUnit(9, ctx.RenderGraph->GetPointShadowMap(m_InputPointShadow)->GetRendererID());
                m_Shader->SetInt("u_PointShadowMaps", 9);

                std::vector<float> farPlanes(MAX_SHADOW_LIGHTS, 100.0f);
                for (uint32_t i = 0; i < ctx.PointShadow.Count; ++i)
                    farPlanes[i] = ctx.PointShadow.Lights[i].FarPlane;
                m_Shader->SetFloatArray("u_PointShadowFarPlanes", farPlanes);
            }
            std::vector<glm::mat4> shadowLightsViewProjs;
            for (int i = 0; i < (int)ctx.ShadowViewProjections.size() && i < MAX_SHADOW_LIGHTS; ++i)
                shadowLightsViewProjs.push_back(ctx.ShadowViewProjections[i]);
            m_Shader->SetMat4Array("u_ShadowMatrices", shadowLightsViewProjs);

            // camera
            m_Shader->SetMat4("u_View", ctx.Camera.ViewMatrix);
            m_Shader->SetMat4("u_Projection", ctx.Camera.ProjectionMatrix);
            m_Shader->SetFloat3("u_CameraPos", ctx.Camera.Position);

            // IBL environment
            auto& env = PBRRenderer::GetEnvironment();
            if (env.IrradianceMap)
            {
                env.IrradianceMap->Bind(5);
                m_Shader->SetInt("u_IrradianceMap", 5);
            }
            if (env.PrefilterMap)
            {
                env.PrefilterMap->Bind(6);
                m_Shader->SetInt("u_PrefilterMap", 6);
            }
            if (env.BRDFLUT)
            {
                env.BRDFLUT->Bind(7);
                m_Shader->SetInt("u_BRDFLUT", 7);
            }

            // batch entity
            
            for (auto& proxy : ctx.Renderables)
            {
                if (!proxy.Visible)
                    continue;
                batchBuilder.AddRenderable(proxy);
            }
            batchBuilder.Sort();
            auto& batches = batchBuilder.GetBatches();

            // Fill instance matrices (each batch occupies a contiguous range)
            m_InstanceRenderer->Reset();
            for (auto& batch : batches)
                if (!batch.Matrices.empty())
                    batch.RenderOffset = m_InstanceRenderer->AddInstances(batch.Matrices);

            // Render (state cache retained, draw changed to instanced)
            // State cache
            Ref<VertexArray> curVAO = nullptr;
            Ref<Texture2D> curTex[4] = { nullptr, nullptr, nullptr, nullptr };
            for (auto& batch : batches)
            {
                if (batch.Matrices.empty())
                    continue;

                const auto& key = batch.Key;
                if (key.VAO != curVAO)
                {
                    curVAO = key.VAO;
                    curVAO->Bind();
                }

                // Texture state cache
                if (key.Albedo != curTex[0])
                {
                    curTex[0] = key.Albedo;
                    if (key.Albedo)
                        key.Albedo->Bind(1);
                }
                if (key.Normal != curTex[1])
                {
                    curTex[1] = key.Normal;
                    if (key.Normal)
                        key.Normal->Bind(2);
                }
                if (key.MetallicRoughness != curTex[2])
                {
                    curTex[2] = key.MetallicRoughness;
                    if (key.MetallicRoughness)
                        key.MetallicRoughness->Bind(3);
                }
                if (key.AO != curTex[3])
                {
                    curTex[3] = key.AO;
                    if (key.AO)
                        key.AO->Bind(4);
                }

                // material factor
                m_Shader->SetFloat3("u_AlbedoFactor", glm::vec3(key.AlbedoFactor));
                m_Shader->SetFloat("u_MetallicFactor", key.MetallicRoughnessFactor.x);
                m_Shader->SetFloat("u_RoughnessFactor", key.MetallicRoughnessFactor.y);
                m_Shader->SetFloat("u_AOStrength", key.AOStrength);

                m_InstanceRenderer->BindRange(batch.RenderOffset, (uint32_t)batch.Matrices.size());
                RenderCommand::DrawIndexedInstanced(key.VAO, key.IndexCount, (uint32_t)batch.Matrices.size());
            }

            // Skybox
            if (env.Skybox && env.SkyboxCubeVAO)
            {
                env.SkyboxShader->Bind();
                env.SkyboxShader->SetMat4("u_Projection", ctx.Camera.ProjectionMatrix);
                glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(ctx.Camera.ViewMatrix));
                env.SkyboxShader->SetMat4("u_View", viewNoTranslate);
                env.Skybox->Bind(0);
                env.SkyboxShader->SetInt("u_Skybox", 0);

                RenderCommand::SetDepthFunc(RendererAPI::DepthFunc::LessEqual);
                env.SkyboxCubeVAO->Bind();
                RenderCommand::DrawArrays(RendererAPI::DrawMode::Triangles, 36);
                RenderCommand::SetDepthFunc(RendererAPI::DepthFunc::Less);
            }

            ctx.RenderGraph->GetFrameBuffer(output)->UnBind();
        }

        ctx.State.DrawCalls = (uint32_t)batchBuilder.GetBatches().size();
        ctx.State.MeshCount = (uint32_t)batchBuilder.GetBatches().size();
    }

    void MainPass::SetInputShadow(TextureHandle& handle)
    {
        m_InputShadow = handle;
    }

    void MainPass::SetInputPointShadow(TextureHandle& handle)
    {
        m_InputPointShadow = handle;
    }

}