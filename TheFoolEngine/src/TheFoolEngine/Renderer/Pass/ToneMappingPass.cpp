#include "tfpch.h"
#include "ToneMappingPass.h"

#include "../RenderUtil.h"
#include "../RenderCommand.h"

namespace TheFoolEngine
{

    ToneMappingPass::ToneMappingPass(Ref<Shader> shader, Ref<FrameBuffer> output)
        : m_Shader(shader), m_OutputFBO(output)
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
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::Off);

        m_OutputFBO->Bind();
        m_Shader->Bind();

        ctx.ResourcePool->GetTexture(m_Input.PoolIndex)->Bind(0);
        
        m_Shader->SetFloat("u_Exposure", m_Exposure);
        m_Shader->SetInt("u_HDRColor", m_HDRColor);

        auto quadVAO =  RenderUtil::Get()->CreateFullscreenQuadVAO();
        quadVAO->Bind();
        RenderCommand::DrawIndexed(quadVAO, 6);
    }

}
