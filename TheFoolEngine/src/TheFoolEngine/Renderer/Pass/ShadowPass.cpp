#include "tfpch.h"
#include "ShadowPass.h"

#include "../PBRRenderer.h"
#include "../RenderCommand.h"
#include "../Shader.h"
#include "../ShadowRenderer.h"
#include "../RenderGraph.h"

namespace TheFoolEngine
{
    //void ShadowPass::Execute(RenderContext& context)
    //{
    //    uint32_t layerCount = (uint32_t)PBRRenderer::GetShadowViewProjections().size();
    //    if (layerCount == 0)
    //        return;

    //    PBRRenderer::GetShadowFBO()->Bind();
    //    PBRRenderer::GetDepthOnlyShader()->Bind();

    //    RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
    //    RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);

    //    for (uint32_t layer = 0; layer < layerCount; ++layer)
    //    {
    //        if (layer >= MAX_SHADOW_LIGHTS)
    //            break;

    //        PBRRenderer::GetShadowFBO()->AttachLayer(layer);
    //        RenderCommand::SetClearColor({ 1.0f,1.0f, 1.0f, 1.0f });
    //        RenderCommand::Clear();
    //        PBRRenderer::GetDepthOnlyShader()->SetMat4("u_LightViewProjection", PBRRenderer::GetShadowViewProjections()[layer]);

    //        for (auto& proxy : context.Renderables)
    //        {
    //            if (!proxy.Visible)
    //                continue;

    //            auto& modelData = proxy.Model->GetModelData();
    //            auto& vas = proxy.Model->GetVertexArray();
    //            auto& meshes = modelData.Meshes;

    //            for (std::size_t i = 0; i < vas.size(); ++i)
    //            {
    //                glm::mat4 model = proxy.Transform * meshes[i].NodeTransform;
    //                PBRRenderer::GetDepthOnlyShader()->SetMat4("u_Model", model);
    //                vas[i]->Bind();
    //                RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
    //            }
    //        }
    //    }

    //    PBRRenderer::GetDepthOnlyShader()->Unbind();
    //}

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
        uint32_t layerCount = (uint32_t)context.ShadowRenderer->GetViewProjections().size();
        if (layerCount == 0)
            return;

        // context.ShadowRenderer->GetShadowFBO()->Bind();
        context.RenderGraph->GetFrameBuffer(context.ShadowRenderer->GetShadowFBOHandle())->Bind();
        context.ShadowRenderer->GetDepthShader()->Bind();

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);

        for (uint32_t layer = 0; layer < layerCount; ++layer)
        {
            if (layer >= MAX_SHADOW_LIGHTS)
                break;

            context.RenderGraph->GetFrameBuffer(context.ShadowRenderer->GetShadowFBOHandle())->AttachLayer(layer);
            RenderCommand::SetClearColor({ 1.0f,1.0f, 1.0f, 1.0f });
            RenderCommand::Clear();
            context.ShadowRenderer->GetDepthShader()->SetMat4("u_LightViewProjection", context.ShadowRenderer->GetViewProjections()[layer]);

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
                    context.ShadowRenderer->GetDepthShader()->SetMat4("u_Model", model);
                    vas[i]->Bind();
                    RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
                }
            }
        }

        context.RenderGraph->GetFrameBuffer(context.ShadowRenderer->GetShadowFBOHandle())->UnBind();
        context.ShadowRenderer->GetDepthShader()->Unbind();
    }
    // ====================================================================== //

    //void PointShadowPass::Execute(RenderContext& context)
    //{
    //    if (!PBRRenderer::GetPointShadowData().DepthMap)
    //        return;

    //    PBRRenderer::GetPointShadowData().DepthMap->Bind();
    //    PBRRenderer::GetPointShadowData().DepthShader->Bind();

    //    RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
    //    RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);
    //    RenderCommand::SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    //    RenderCommand::Clear();

    //    for (uint32_t lightIndex = 0; lightIndex < PBRRenderer::GetPointShadowData().Count; ++lightIndex)
    //    {
    //        const auto& light = PBRRenderer::GetPointShadowData().Lights[lightIndex];

    //        for (int face = 0; face < 6; ++face)
    //        {
    //            PBRRenderer::GetPointShadowData().DepthMap->BindFace(lightIndex, face);
    //            RenderCommand::Clear();
    //            PBRRenderer::GetPointShadowData().DepthShader->SetMat4("u_LightViewProjection",
    //                light.ShadowProj * light.ShadowViews[face]);
    //            PBRRenderer::GetPointShadowData().DepthShader->SetFloat3("u_LightPos", light.LightPosition);
    //            PBRRenderer::GetPointShadowData().DepthShader->SetFloat("u_FarPlane", light.FarPlane);

    //            // Traverse renderables to draw depth
    //            for (auto& proxy : context.Renderables)
    //            {
    //                if (!proxy.Visible)
    //                    continue;

    //                auto& modelData = proxy.Model->GetModelData();
    //                auto& vas = proxy.Model->GetVertexArray();
    //                auto& meshes = modelData.Meshes;
    //                for (std::size_t i = 0; i < vas.size(); ++i)
    //                {
    //                    glm::mat4 model = proxy.Transform * meshes[i].NodeTransform;
    //                    PBRRenderer::GetPointShadowData().DepthShader->SetMat4("u_Model", model);
    //                    vas[i]->Bind();
    //                    RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
    //                }
    //            }
    //        }
    //    }

    //    PBRRenderer::GetPointShadowData().DepthMap->Unbind();
    //}

    void PointShadowPass::Execute(RenderContext& context)
    {
        auto shadowMap = context.RenderGraph->GetPointShadowMap(context.ShadowRenderer->GetPointShadowHandle());
        if (!shadowMap)
            return;

        auto shader = context.ShadowRenderer->GetPointDepthShader();

        shadowMap->Bind();
        shader->Bind();

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);
        RenderCommand::SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        RenderCommand::Clear();

        for (uint32_t lightIndex = 0; lightIndex < context.ShadowRenderer->GetPointShadowData().Count; ++lightIndex)
        {
            const auto& light = context.ShadowRenderer->GetPointShadowData().Lights[lightIndex];

            for (int face = 0; face < 6; ++face)
            {
                shadowMap->BindFace(lightIndex, face);
                RenderCommand::Clear();
                shader->SetMat4("u_LightViewProjection",
                    light.ShadowProj * light.ShadowViews[face]);
                shader->SetFloat3("u_LightPos", light.LightPosition);
                shader->SetFloat("u_FarPlane", light.FarPlane);

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
                        shader->SetMat4("u_Model", model);
                        vas[i]->Bind();
                        RenderCommand::DrawIndexed(vas[i], (uint32_t)meshes[i].indices.size());
                    }
                }
            }
        }

        shadowMap->Unbind();
        shader->Unbind();
    }

}
