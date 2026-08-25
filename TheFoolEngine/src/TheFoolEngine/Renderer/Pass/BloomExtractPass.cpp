#include "tfpch.h"
#include "BloomExtractPass.h"

#include "../RenderUtil.h"
#include "../RenderCommand.h"

namespace TheFoolEngine
{

    BloomExtractPass::BloomExtractPass(Ref<Shader> shader, Ref<FrameBuffer> output)
        :m_Shader(shader), m_OutputFBO(output)
    {
    }

    void BloomExtractPass::SetInput(TextureHandle input)
    {
        m_Input = input;
    }

    void BloomExtractPass::SetOutput(TextureHandle output)
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
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::Off);

        m_OutputFBO->Bind();
        m_Shader->Bind();

        m_Shader->SetFloat("u_Threshold", m_Threshold);
        ctx.ResourcePool->GetTexture(m_Input.PoolIndex)->Bind(0);
        auto quadVAO = RenderUtil::Get()->CreateFullscreenQuadVAO();
        quadVAO->Bind();
        RenderCommand::DrawIndexed(quadVAO, 6);
    }

}
