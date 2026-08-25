#include "tfpch.h"
#include "BloomBlurPass.h"

#include "../RenderCommand.h"
#include "../RenderUtil.h"

namespace TheFoolEngine
{
    BloomBlurPass::BloomBlurPass(Ref<Shader> shader, Ref<FrameBuffer> output)
        : m_Shader(shader), m_OutputFBO(output)
    {
        m_Direction = glm::vec2(0.0f);
    }

    void BloomBlurPass::SetDirection(const glm::vec2& dir)
    {
        m_Direction = dir;
    }

    void BloomBlurPass::SetInput(TextureHandle input)
    {
        m_Input = input;
    }

    void BloomBlurPass::SetOutput(TextureHandle output)
    {
        m_Output = output;
    }

    std::vector<TextureHandle>& BloomBlurPass::GetInputs()
    {
        m_Inputs = { m_Input };
        return m_Inputs;
    }

    std::vector<TextureHandle>& BloomBlurPass::GetOutputs()
    {
        m_Outputs = { m_Output };
        return m_Outputs;
    }

    void BloomBlurPass::Execute(RenderContext& ctx)
    {
        RenderCommand::SetDepthTest(RendererAPI::DepthTest::Off);
        RenderCommand::SetDepthWrite(RendererAPI::DepthWrite::Off);

        m_OutputFBO->Bind();
        m_Shader->Bind();
        m_Shader->SetFloat2("u_Direction", m_Direction);
        
        ctx.ResourcePool->GetTexture(m_Input.PoolIndex)->Bind(0);

        auto quadVAO = RenderUtil::Get()->CreateFullscreenQuadVAO();
        quadVAO->Bind();
        RenderCommand::DrawIndexed(quadVAO, 6);
    }

}
