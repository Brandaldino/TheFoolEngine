#include "tfpch.h"
#include "BloomExtractPass.h"

#include "../RenderUtil.h"
#include "../RenderCommand.h"
#include "../RenderGraph.h"

namespace TheFoolEngine
{

    BloomExtractPass::BloomExtractPass(Ref<Shader> shader)
        :m_Shader(shader)
    {
    }

    void BloomExtractPass::SetInput(TextureHandle& input)
    {
        m_Input = input;
    }

    void BloomExtractPass::SetOutput(TextureHandle& output)
    {
        m_Output = output;
    }

    std::vector<TextureHandle>& BloomExtractPass::GetInputs()
    {
        m_Inputs = { m_Input };
        return m_Inputs;
    }

    std::vector<TextureHandle>& BloomExtractPass::GetOutputs()
    {
        m_Outputs = { m_Output };
        return m_Outputs;
    }

    void BloomExtractPass::Execute(RenderContext& ctx)
    {
        RenderCommand::SetDepthTest(RendererAPI::DepthTest::Off);

        m_Shader->Bind();

        for(auto& output : m_Outputs)
        {
            ctx.RenderGraph->GetFrameBuffer(output)->Bind();
            ctx.RenderGraph->GetTexture(m_Input)->Bind(0);

            m_Shader->SetFloat("u_Threshold", m_Threshold);
            auto quadVAO = RenderUtil::Get()->CreateFullscreenQuadVAO();
            quadVAO->Bind();
            RenderCommand::DrawIndexed(quadVAO, 6);

            ctx.RenderGraph->GetFrameBuffer(output)->UnBind();
        }
    }

}
