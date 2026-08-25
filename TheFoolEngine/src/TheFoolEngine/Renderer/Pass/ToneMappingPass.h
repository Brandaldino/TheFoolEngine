#pragma once

#include "Pass.h"
#include "../Shader.h"

namespace TheFoolEngine
{
    class ToneMappingPass : public Pass
    {
    public:
        ToneMappingPass(Ref<Shader> shader, Ref<FrameBuffer> output);

        virtual void SetInput(TextureHandle input) override;
        virtual void SetOutput(TextureHandle output) override;

        virtual std::vector<TextureHandle>& GetInputs() override;
        virtual std::vector<TextureHandle>& GetOutputs() override;

        virtual void Execute(RenderContext& ctx) override;
        virtual std::string& GetPassName() { return m_PassName; };
    public:
        std::string m_PassName = "ToneMappingPass"; // for debug
    private:
        Ref<Shader> m_Shader;
        Ref<FrameBuffer> m_OutputFBO;
        TextureHandle m_Input, m_Output;
        std::vector<TextureHandle> m_Inputs, m_Outputs;

        float m_Exposure = 1.0f;
        int m_HDRColor = 0;
    };
}