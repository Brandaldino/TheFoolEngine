#include "tfpch.h"
#include "ShadowPass.h"

#include "../PBRRenderer.h"
#include "../PointShadowMap.h"
#include "../RenderCommand.h"
#include "../Shader.h"
#include "../ShadowRenderer.h"
#include "../RenderGraph.h"

namespace TheFoolEngine
{

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
        uint32_t layerCount = (uint32_t)context.ShadowViewProjections.size();
        if (layerCount == 0)
            return;

        context.RenderGraph->GetFrameBuffer(m_Output)->Bind();
        context.ShadowRenderer->GetDepthShader()->Bind();

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::On);

        for (uint32_t layer = 0; layer < layerCount; ++layer)
        {
            if (layer >= MAX_SHADOW_LIGHTS)
                break;

            context.RenderGraph->GetFrameBuffer(m_Output)->AttachLayer(layer);
            RenderCommand::SetClearColor({ 1.0f,1.0f, 1.0f, 1.0f });
            RenderCommand::Clear();
            context.ShadowRenderer->GetDepthShader()->SetMat4("u_LightViewProjection", context.ShadowViewProjections[layer]);

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

        context.RenderGraph->GetFrameBuffer(m_Output)->UnBind();
        context.ShadowRenderer->GetDepthShader()->Unbind();
    }
    // ====================================================================== //

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

        for (uint32_t lightIndex = 0; lightIndex < context.PointShadow.Count; ++lightIndex)
        {
            const auto& light = context.PointShadow.Lights[lightIndex];

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
