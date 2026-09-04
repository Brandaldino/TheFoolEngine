#include "tfpch.h"
#include "BloomBlurPass.h"

#include "../RenderCommand.h"
#include "../RenderUtil.h"
#include "../RenderGraph.h"

namespace TheFoolEngine
{
    BloomBlurPass::BloomBlurPass(Ref<Shader> shader)
        : m_Shader(shader)
    {
        m_Direction = glm::vec2(0.0f);
    }

    void BloomBlurPass::SetDirection(const glm::vec2& dir)
    {
        m_Direction = dir;
    }

    void BloomBlurPass::SetInput(TextureHandle& input)
    {
        m_Input = input;
    }

    void BloomBlurPass::SetOutput(TextureHandle& output)
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

        m_Shader->Bind();
        
        for (auto& output : m_Outputs)
        {
            ctx.RenderGraph->GetFrameBuffer(output)->Bind();

            ctx.RenderGraph->GetTexture(m_Input)->Bind(0);

            m_Shader->SetFloat2("u_Direction", m_Direction);

            auto quadVAO = RenderUtil::Get()->CreateFullscreenQuadVAO();
            quadVAO->Bind();
            RenderCommand::DrawIndexed(quadVAO, 6);

            ctx.RenderGraph->GetFrameBuffer(output)->UnBind();
        }
    }

}
