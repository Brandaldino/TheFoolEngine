#include "tfpch.h"
#include "ShadowPass.h"

#include "../PBRRenderer.h"
#include "../PointShadowMap.h"
#include "../RenderCommand.h"
#include "../Shader.h"
#include "../RenderGraph.h"
#include "../Frustum.h"
#include "../BatchSystem/BatchBuilder.h"
#include "../InstanceRenderer.h"

namespace TheFoolEngine
{
    ShadowPass::ShadowPass(Ref<Shader> shader)
        :m_Shader(shader)
    {
        m_InstanceRenderer = CreateScope<InstanceRenderer>(s_MaxInstances);
    }

    void ShadowPass::SetOutput(TextureHandle& handle)
    {
        m_Output = handle;
    }

    std::vector<TextureHandle>& ShadowPass::GetOutputs()
    {
        m_Outputs = { m_Output };
        return m_Outputs;
    }

    void ShadowPass::Execute(RenderContext& context)
    {
        TF_PROFILE_FUNCTION();

        uint32_t layerCount = (uint32_t)context.ShadowViewProjections.size();
        if (layerCount == 0)
            return;

        context.RenderGraph->GetFrameBuffer(m_Output)->Bind();
        m_Shader->Bind();

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);

        for (uint32_t layer = 0; layer < layerCount; ++layer)
        {
            if (layer >= MAX_SHADOW_LIGHTS)
                break;

            context.RenderGraph->GetFrameBuffer(m_Output)->AttachLayer(layer);
            RenderCommand::SetClearColor({ 1.0f,1.0f, 1.0f, 1.0f });
            RenderCommand::Clear();
            m_Shader->SetMat4("u_LightViewProjection", context.ShadowViewProjections[layer]);

            Frustum layerFrustum;
            layerFrustum.Extract(context.ShadowViewProjections[layer]);

            // Per layer (cull → batch → fill → render)
            BatchBuilder batchBuilder;
            for (auto& proxy : context.ShadowCasters)   // Shadow culling: re-cull per cascade/layer
            {
                if (!proxy.Visible)
                    continue;

                if (!layerFrustum.Intersects(proxy.BoundsCenter, proxy.BoundsHalfExtents))
                    continue;

                batchBuilder.AddRenderable(proxy);
            }
            batchBuilder.Sort();
            auto& batches = batchBuilder.GetBatches();

            m_InstanceRenderer->Reset();
            for (auto& batch : batches)
                batch.RenderOffset = m_InstanceRenderer->AddInstances(batch.Matrices);

            for (auto& batch : batches)
            {
                if (batch.Matrices.empty())
                    continue;
                const auto& key = batch.Key;
                key.VAO->Bind();
                m_InstanceRenderer->BindRange(batch.RenderOffset, (uint32_t)batch.Matrices.size());
                RenderCommand::DrawIndexedInstanced(key.VAO, key.IndexCount, (uint32_t)batch.Matrices.size());
            }

            //BatchBuilder batchBuilder;
            //for (auto& proxy : context.ShadowCasters)   // Shadow culling: re-cull per cascade/layer
            //{
            //    if (!proxy.Visible)
            //        continue;

            //    if (!layerFrustum.Intersects(proxy.BoundsCenter, proxy.BoundsHalfExtents))
            //        continue;

            //    auto& modelData = proxy.Model->GetModelData();
            //    auto& vas = proxy.Model->GetVertexArray();
            //    auto& meshes = modelData.Meshes;

            //    for (std::size_t i = 0; i < vas.size(); ++i)
            //    {
            //        glm::mat4 model = proxy.Transform * meshes[i].NodeTransform;
            //        m_Shader->SetMat4("u_Model", model);
            //        vas[i]->Bind();
            //        RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
            //    }
            //}
            //batchBuilder.Sort();

        }

        context.RenderGraph->GetFrameBuffer(m_Output)->UnBind();
        m_Shader->Unbind();
    }
    // ====================================================================== //

    PointShadowPass::PointShadowPass(Ref<Shader> shader)
        :m_Shader(shader)
    {
        m_InstanceRenderer = CreateScope<InstanceRenderer>(s_MaxInstances);
    }

    void PointShadowPass::SetOutput(TextureHandle& handle)
    {
        m_Output = handle;
    }

    std::vector<TextureHandle>& PointShadowPass::GetOutputs()
    {
        m_Outputs = { m_Output };
        return m_Outputs;
    }

    void PointShadowPass::Execute(RenderContext& context)
    {
        TF_PROFILE_FUNCTION();

        auto shadowMap = context.RenderGraph->GetPointShadowMap(m_Output);
        if (!shadowMap)
            return;

        shadowMap->Bind();
        m_Shader->Bind();

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);
        RenderCommand::SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        RenderCommand::Clear();

        for (uint32_t lightIndex = 0; lightIndex < context.PointShadow.Count; ++lightIndex)
        {
            const auto& light = context.PointShadow.Lights[lightIndex];

            for (int face = 0; face < 6; ++face)
            {
                shadowMap->BindFace(lightIndex, face);
                RenderCommand::Clear();
                m_Shader->SetMat4("u_LightViewProjection",
                    light.ShadowProj * light.ShadowViews[face]);
                m_Shader->SetFloat3("u_LightPos", light.LightPosition);
                m_Shader->SetFloat("u_FarPlane", light.FarPlane);

                Frustum faceFrustum;
                faceFrustum.Extract(light.ShadowProj * light.ShadowViews[face]);

                BatchBuilder batchBuilder;
                // Traverse renderables to draw depth
                for (auto& proxy : context.ShadowCasters)   // Shadow culling: re-cull per cascade/layer
                {
                    if (!proxy.Visible)
                        continue;

                    if (!faceFrustum.Intersects(proxy.BoundsCenter, proxy.BoundsHalfExtents))
                        continue;

                    batchBuilder.AddRenderable(proxy);
                }
                batchBuilder.Sort();
                auto& batches = batchBuilder.GetBatches();

                m_InstanceRenderer->Reset();
                for (auto& batch : batches)
                    batch.RenderOffset = m_InstanceRenderer->AddInstances(batch.Matrices);

                for (auto& batch : batches)
                {
                    if (batch.Matrices.empty())
                        continue;
                    const auto& key = batch.Key;
                    key.VAO->Bind();
                    m_InstanceRenderer->BindRange(batch.RenderOffset, (uint32_t)batch.Matrices.size());
                    RenderCommand::DrawIndexedInstanced(key.VAO, key.IndexCount, (uint32_t)batch.Matrices.size());
                }

                //for (auto& proxy : context.ShadowCasters)
                //{
                //    if (!proxy.Visible)
                //        continue;

                //    if (!faceFrustum.Intersects(proxy.BoundsCenter, proxy.BoundsHalfExtents))
                //        continue;

                //    auto& modelData = proxy.Model->GetModelData();
                //    auto& vas = proxy.Model->GetVertexArray();
                //    auto& meshes = modelData.Meshes;
                //    for (std::size_t i = 0; i < vas.size(); ++i)
                //    {
                //        glm::mat4 model = proxy.Transform * meshes[i].NodeTransform;
                //        m_Shader->SetMat4("u_Model", model);
                //        vas[i]->Bind();
                //        RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
                //    }
                //}
            }
        }

        shadowMap->Unbind();
        m_Shader->Unbind();
    }

}
