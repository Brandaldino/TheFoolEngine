#include "tfpch.h"
#include "ToneMappingPass.h"

#include "../RenderUtil.h"
#include "../RenderCommand.h"
#include "../RenderGraph.h"

namespace TheFoolEngine
{

    ToneMappingPass::ToneMappingPass(Ref<Shader> shader)
        : m_Shader(shader)
    {
    }

    void ToneMappingPass::SetInput(TextureHandle input)
    {
        m_Input = input;
    }

    void ToneMappingPass::SetOutput(TextureHandle output)
    {
        m_Output = output;
    }

    std::vector<TextureHandle>& ToneMappingPass::GetInputs()
    {
        m_Inputs = { m_Input };
        return m_Inputs;
    }

    std::vector<TextureHandle>& ToneMappingPass::GetOutputs()
    {
        m_Outputs = { m_Output };
        return m_Outputs;
    }

    void ToneMappingPass::Execute(RenderContext& ctx)
    {
        RenderCommand::SetDepthTest(RendererAPI::DepthTest::Off);

        m_Shader->Bind();

        for (auto& output : m_Outputs)
        {
            ctx.RenderGraph->GetFrameBuffer(output)->Bind();

            ctx.RenderGraph->GetTexture(m_Input)->Bind(0);

            m_Shader->SetFloat("u_Exposure", m_Exposure);
            m_Shader->SetInt("u_HDRColor", m_HDRColor);

            auto quadVAO = RenderUtil::Get()->CreateFullscreenQuadVAO();
            quadVAO->Bind();
            RenderCommand::DrawIndexed(quadVAO, 6);

            ctx.RenderGraph->GetFrameBuffer(output)->UnBind();
        }
    }

}
