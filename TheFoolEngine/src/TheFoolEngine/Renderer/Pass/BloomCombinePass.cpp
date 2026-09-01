#include "tfpch.h"
#include "BloomCombinePass.h"

#include "../RenderCommand.h"
#include "../RenderUtil.h"
#include "../RenderGraph.h"

namespace TheFoolEngine
{

    BloomCombinePass::BloomCombinePass(Ref<Shader> shader)
        :m_Shader(shader)
    {
    }

    void BloomCombinePass::SetOutput(TextureHandle output)
    {
        m_Output = output;
    }

    void BloomCombinePass::SetInputHDR(TextureHandle hdr)
    {
        m_HDR = hdr;
    }

    void BloomCombinePass::SetInputBloom(TextureHandle bloom)
    {
        m_Bloom = bloom;
    }

    std::vector<TextureHandle>& BloomCombinePass::GetInputs()
    {
        m_Inputs = { m_HDR, m_Bloom };
        return m_Inputs;
    }

    std::vector<TextureHandle>& BloomCombinePass::GetOutputs()
    {
        m_Outputs = { m_Output };
        return m_Outputs;
    }

    void BloomCombinePass::Execute(RenderContext& ctx)
    {
        RenderCommand::SetDepthTest(RendererAPI::DepthTest::Off);

        m_Shader->Bind();

        for (auto& output : m_Outputs)
        {
            ctx.RenderGraph->GetFrameBuffer(output)->Bind();

            ctx.RenderGraph->GetTexture(m_HDR)->Bind(0);
            ctx.RenderGraph->GetTexture(m_Bloom)->Bind(1);

            m_Shader->SetInt("u_HDRColor", m_HDRColor);
            m_Shader->SetInt("u_BloomBlur", m_BloomBlur);
            m_Shader->SetFloat("u_Intensity", m_Intensity);

            auto quadVAO = RenderUtil::Get()->CreateFullscreenQuadVAO();
            quadVAO->Bind();
            RenderCommand::DrawIndexed(quadVAO, 6);

            ctx.RenderGraph->GetFrameBuffer(output)->UnBind();
        }
    }

}
